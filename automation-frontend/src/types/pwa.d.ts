/// <reference types="vite/client" />
/// <reference types="vite-plugin-pwa/client" />

declare module 'virtual:pwa-register/vue' {
  import type { Ref } from 'vue'
  
  export interface RegisterSWOptions {
    onRegisteredSW?: (swUrl: string, registration: ServiceWorkerRegistration | undefined) => void
    onRegisterError?: (error: any) => void
    onOfflineReady?: () => void
    onNeedRefresh?: () => void
  }
  
  export interface PWARegisterReturn {
    needRefresh: Ref<boolean>
    updateServiceWorker: (reloadPage?: boolean) => Promise<void>
    offlineReady: Ref<boolean>
  }
  
  export function useRegisterSW(options?: RegisterSWOptions): PWARegisterReturn
}