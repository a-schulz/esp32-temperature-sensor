import { ref, computed, type Ref } from 'vue'
import { useRegisterSW } from 'virtual:pwa-register/vue'

// Define types for PWA functionality
interface BeforeInstallPromptEvent extends Event {
  readonly platforms: string[]
  readonly userChoice: Promise<{
    outcome: 'accepted' | 'dismissed'
    platform: string
  }>
  prompt(): Promise<void>
}

export const usePWA = () => {
  const deferredPrompt = ref<BeforeInstallPromptEvent | null>(null)
  const isInstallable = ref(false)
  const isInstalled = ref(false)
  const isOnline = ref(navigator.onLine)

  // Service Worker registration and update handling
  const {
    needRefresh,
    updateServiceWorker,
    offlineReady,
  } = useRegisterSW({
    onRegisteredSW(swUrl: string, registration: ServiceWorkerRegistration | undefined) {
      console.log('Service Worker registered:', swUrl)
      
      // Check for updates periodically
      if (registration) {
        setInterval(() => {
          registration.update()
        }, 60000) // Check every minute
      }
    },
    onRegisterError(error: any) {
      console.error('Service Worker registration error:', error)
    },
    onOfflineReady() {
      console.log('App ready to work offline')
    },
    onNeedRefresh() {
      console.log('New content available, will refresh...')
    },
  })

  // Check if app is already installed
  const checkInstallStatus = () => {
    if (window.matchMedia('(display-mode: standalone)').matches || 
        (window.navigator as any).standalone ||
        document.referrer.includes('android-app://')) {
      isInstalled.value = true
    }
  }

  // Listen for the beforeinstallprompt event
  const setupInstallPrompt = () => {
    window.addEventListener('beforeinstallprompt', (e: Event) => {
      e.preventDefault()
      deferredPrompt.value = e as BeforeInstallPromptEvent
      isInstallable.value = true
    })

    window.addEventListener('appinstalled', () => {
      console.log('PWA installed successfully')
      isInstalled.value = true
      isInstallable.value = false
      deferredPrompt.value = null
    })
  }

  // Install the PWA
  const installPWA = async (): Promise<boolean> => {
    if (!deferredPrompt.value) return false

    try {
      await deferredPrompt.value.prompt()
      const choiceResult = await deferredPrompt.value.userChoice
      
      if (choiceResult.outcome === 'accepted') {
        console.log('User accepted the install prompt')
        return true
      } else {
        console.log('User dismissed the install prompt')
        return false
      }
    } catch (error) {
      console.error('Error during PWA installation:', error)
      return false
    } finally {
      deferredPrompt.value = null
      isInstallable.value = false
    }
  }

  // Handle online/offline status
  const setupNetworkListeners = () => {
    window.addEventListener('online', () => {
      isOnline.value = true
    })

    window.addEventListener('offline', () => {
      isOnline.value = false
    })
  }

  // Share API support
  const canShare = computed(() => {
    return 'share' in navigator
  })

  const shareApp = async (data?: {
    title?: string
    text?: string
    url?: string
  }) => {
    if (!canShare.value) return false

    try {
      await navigator.share({
        title: data?.title || 'Temperature Sensor Dashboard',
        text: data?.text || 'Überwache Temperatur und Luftfeuchtigkeit in Echtzeit',
        url: data?.url || window.location.href,
      })
      return true
    } catch (error) {
      console.error('Error sharing:', error)
      return false
    }
  }

  // Initialize PWA functionality
  const initPWA = () => {
    checkInstallStatus()
    setupInstallPrompt()
    setupNetworkListeners()
  }

  return {
    // State
    isInstallable,
    isInstalled,
    isOnline,
    needRefresh,
    offlineReady,
    canShare,

    // Actions
    installPWA,
    updateServiceWorker,
    shareApp,
    initPWA,
  }
}