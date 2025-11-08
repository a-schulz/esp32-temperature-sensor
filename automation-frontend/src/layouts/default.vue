<template>
  <v-app>
    <v-app-bar color="primary" dark flat height="80">
      <v-app-bar-title class="text-h5 font-weight-bold ml-4">
        Sensoranalyse
      </v-app-bar-title>

      <!-- Connection Status -->
      <v-chip
        class="ma-2 text-body-1"
        :color="connectionStatus.color"
        :prepend-icon="connectionStatus.icon"
        size="large"
      >
        {{ connectionStatus.text }}
      </v-chip>

      <!-- Dark Mode Toggle -->
      <v-btn class="ml-2 mr-4" icon size="large" @click="toggleDarkMode">
        <v-icon size="28">
          {{ darkMode ? 'mdi-brightness-7' : 'mdi-brightness-4' }}
        </v-icon>
      </v-btn>
    </v-app-bar>

    <v-main class="senior-friendly-main">
      <v-container class="pa-6" fluid>
        <router-view />
      </v-container>
    </v-main>

    <!-- Footer with last update time -->
    <v-footer class="text-center pa-4" color="surface-variant">
      Viele liebe Grüße, euer Alex
    </v-footer>
  </v-app>
</template>

<script lang="ts" setup>
  import { useStorage } from '@vueuse/core'
  import { computed, onMounted, ref } from 'vue'
  import { useTheme } from 'vuetify'

  const theme = useTheme()
  const darkMode = useStorage('darkMode', false)
  const isOnline = ref(navigator.onLine)

  // Connection status
  const connectionStatus = computed(() => {
    if (!isOnline.value) {
      return {
        color: 'error',
        icon: 'mdi-wifi-off',
        text: 'Offline',
      }
    }
    return {
      color: 'success',
      icon: 'mdi-wifi',
      text: 'Online',
    }
  })

  // Toggle dark mode
  function toggleDarkMode () {
    darkMode.value = !darkMode.value
    theme.global.name.value = darkMode.value ? 'dark' : 'light'
  }

  // Listen for online/offline events
  function handleOnline () {
    isOnline.value = true
  }
  function handleOffline () {
    isOnline.value = false
  }

  onMounted(() => {
    // Set initial theme
    theme.global.name.value = darkMode.value ? 'dark' : 'light'

    // Listen for online/offline events
    window.addEventListener('online', handleOnline)
    window.addEventListener('offline', handleOffline)
  })
</script>

<style lang="scss">
    .senior-friendly-main {
        min-height: calc(100vh - 160px);

        // Large text for seniors
        .text-h3 {
            font-size: 2.5rem !important;
            font-weight: 500;
            line-height: 1.2;
        }

        .text-h4 {
            font-size: 2rem !important;
            font-weight: 500;
            line-height: 1.3;
        }

        .text-h5 {
            font-size: 1.5rem !important;
            font-weight: 500;
            line-height: 1.4;
        }

        .temperature-value {
            font-size: 3rem !important;
            font-weight: bold;
            line-height: 1;
        }

        .humidity-value {
            font-size: 2.5rem !important;
            font-weight: bold;
            line-height: 1;
        }

        // High contrast colors
        .v-card {
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.12) !important;
        }

        // Larger touch targets
        .v-btn {
            min-height: 48px;
            min-width: 48px;

            &.v-btn--size-large {
                min-height: 56px;
                min-width: 56px;
            }
        }

        // Better spacing for readability
        .sensor-card {
            padding: 24px;
            margin: 16px 0;
            border-radius: 12px !important;
        }

        // Alert styling
        .v-alert {
            font-size: 1.1rem;
            line-height: 1.4;

            .v-alert__content {
                font-weight: 500;
            }
        }
    }

    // Reduce motion for users who prefer it
    @media (prefers-reduced-motion: reduce) {
        * {
            animation-duration: 0.01ms !important;
            animation-iteration-count: 1 !important;
            transition-duration: 0.01ms !important;
        }
    }

    // High contrast mode
    @media (prefers-contrast: high) {
        .v-card {
            border: 2px solid currentColor !important;
        }

        .v-btn {
            border: 2px solid currentColor !important;
        }
    }

    // Large text scaling support
    @media (min-resolution: 192dpi) {
        .temperature-value {
            font-size: 3.5rem !important;
        }

        .humidity-value {
            font-size: 3rem !important;
        }
    }
</style>
