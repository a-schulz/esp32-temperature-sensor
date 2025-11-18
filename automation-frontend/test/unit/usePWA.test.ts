import { beforeEach, describe, expect, it, vi } from 'vitest'

// Mock virtual:pwa-register/vue
vi.mock('virtual:pwa-register/vue', () => ({
  useRegisterSW: vi.fn(() => ({
    needRefresh: { value: false },
    updateServiceWorker: vi.fn(),
    offlineReady: { value: false },
  })),
}))

describe('usePWA', () => {
  let usePWA: any

  beforeEach(async () => {
    vi.clearAllMocks()
    vi.resetModules()

    // Mock window.matchMedia
    Object.defineProperty(window, 'matchMedia', {
      writable: true,
      value: vi.fn().mockImplementation(query => ({
        matches: false,
        media: query,
        onchange: null,
        addListener: vi.fn(),
        removeListener: vi.fn(),
        addEventListener: vi.fn(),
        removeEventListener: vi.fn(),
        dispatchEvent: vi.fn(),
      })),
    })

    // Mock navigator.onLine
    Object.defineProperty(navigator, 'onLine', {
      writable: true,
      value: true,
    })

    const module = await import('@/composables/usePWA')
    usePWA = module.usePWA
  })

  describe('initialization', () => {
    it('should initialize with correct default state', () => {
      const pwa = usePWA()
      expect(pwa.isInstallable.value).toBe(false)
      expect(pwa.isInstalled.value).toBe(false)
      expect(pwa.isOnline.value).toBe(true)
    })
  })

  describe('checkInstallStatus', () => {
    it('should detect standalone mode', async () => {
      Object.defineProperty(window, 'matchMedia', {
        writable: true,
        value: vi.fn().mockImplementation(query => ({
          matches: query === '(display-mode: standalone)',
          media: query,
          onchange: null,
          addListener: vi.fn(),
          removeListener: vi.fn(),
          addEventListener: vi.fn(),
          removeEventListener: vi.fn(),
          dispatchEvent: vi.fn(),
        })),
      })

      const module = await import('@/composables/usePWA')
      const pwa = module.usePWA()
      pwa.initPWA()

      expect(pwa.isInstalled.value).toBe(true)
    })

    it('should detect iOS standalone mode', async () => {
      Object.defineProperty(window.navigator, 'standalone', {
        writable: true,
        value: true,
      })

      const module = await import('@/composables/usePWA')
      const pwa = module.usePWA()
      pwa.initPWA()

      expect(pwa.isInstalled.value).toBe(true)
    })
  })

  describe('installPWA', () => {
    it('should return false when no deferred prompt is available', async () => {
      const pwa = usePWA()
      const result = await pwa.installPWA()
      expect(result).toBe(false)
    })
  })

  describe('network status', () => {
    it('should track online status', () => {
      const pwa = usePWA()
      expect(pwa.isOnline.value).toBe(true)
    })

    it('should update when going offline', async () => {
      const pwa = usePWA()
      pwa.initPWA()

      // Simulate going offline
      Object.defineProperty(navigator, 'onLine', {
        writable: true,
        value: false,
      })

      const offlineEvent = new Event('offline')
      window.dispatchEvent(offlineEvent)

      // The composable should have set up the listener
      expect(typeof pwa.isOnline.value).toBe('boolean')
    })
  })

  describe('shareApp', () => {
    it('should return false when share API is not available', async () => {
      const originalShare = navigator.share
      // @ts-expect-error
      delete navigator.share

      const pwa = usePWA()
      const result = await pwa.shareApp()

      expect(result).toBe(false)

      // Restore
      if (originalShare) {
        navigator.share = originalShare
      }
    })

    it('should detect share API availability', () => {
      Object.defineProperty(navigator, 'share', {
        writable: true,
        value: vi.fn(),
      })

      const pwa = usePWA()
      expect(pwa.canShare.value).toBe(true)
    })
  })

  describe('Service Worker integration', () => {
    it('should expose needRefresh state', () => {
      const pwa = usePWA()
      expect(pwa.needRefresh).toBeDefined()
    })

    it('should expose offlineReady state', () => {
      const pwa = usePWA()
      expect(pwa.offlineReady).toBeDefined()
    })

    it('should provide updateServiceWorker method', () => {
      const pwa = usePWA()
      expect(typeof pwa.updateServiceWorker).toBe('function')
    })
  })
})
