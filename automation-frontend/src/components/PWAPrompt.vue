<template>
  <div>
    <!-- Install Prompt Banner -->
    <v-banner
      v-if="isInstallable && !isInstalled"
      color="primary"
      icon="mdi-download"
      lines="two"
      sticky
    >
      <template #text>
        <div>
          <div class="text-subtitle1 font-weight-medium">
            App installieren
          </div>
          <div class="text-body2">
            Installiere die Temperature Sensor App für einen
            schnelleren Zugriff und Offline-Nutzung.
          </div>
        </div>
      </template>
      <template #actions>
        <v-btn
          color="white"
          size="small"
          variant="outlined"
          @click="dismissInstall"
        >
          Später
        </v-btn>
        <v-btn
          color="white"
          :loading="installing"
          size="small"
          variant="flat"
          @click="handleInstall"
        >
          <v-icon start>mdi-download</v-icon>
          Installieren
        </v-btn>
      </template>
    </v-banner>

    <!-- Update Available Banner -->
    <v-banner
      v-if="needRefresh"
      color="info"
      icon="mdi-update"
      lines="two"
      sticky
    >
      <template #text>
        <div>
          <div class="text-subtitle1 font-weight-medium">
            Update verfügbar
          </div>
          <div class="text-body2">
            Eine neue Version der App ist verfügbar. Jetzt
            aktualisieren?
          </div>
        </div>
      </template>
      <template #actions>
        <v-btn
          color="white"
          size="small"
          variant="outlined"
          @click="dismissUpdate"
        >
          Später
        </v-btn>
        <v-btn
          color="white"
          :loading="updating"
          size="small"
          variant="flat"
          @click="handleUpdate"
        >
          <v-icon start>mdi-refresh</v-icon>
          Aktualisieren
        </v-btn>
      </template>
    </v-banner>

    <!-- Offline Status -->
    <v-banner
      v-if="!isOnline"
      color="warning"
      icon="mdi-wifi-off"
      lines="one"
      sticky
    >
      <template #text>
        <div class="text-subtitle1">
          Offline-Modus - Daten werden möglicherweise nicht
          aktualisiert
        </div>
      </template>
    </v-banner>

    <!-- Offline Ready Notification -->
    <v-snackbar v-model="showOfflineReady" color="success" timeout="5000">
      <v-icon start>mdi-check-circle</v-icon>
      App ist bereit für Offline-Nutzung!
      <template #actions>
        <v-btn variant="text" @click="showOfflineReady = false">
          Schließen
        </v-btn>
      </template>
    </v-snackbar>
  </div>
</template>

<script lang="ts" setup>
  import { ref, watch } from 'vue'
  import { usePWA } from '@/composables/usePWA'

  const {
    isInstallable,
    isInstalled,
    isOnline,
    needRefresh,
    offlineReady,
    installPWA,
    updateServiceWorker,
  } = usePWA()

  const installing = ref(false)
  const updating = ref(false)
  const showOfflineReady = ref(false)
  const installDismissed = ref(false)
  const updateDismissed = ref(false)

  // Handle install
  async function handleInstall () {
    installing.value = true
    try {
      const success = await installPWA()
      if (success) {
        console.log('App installed successfully')
      }
    } finally {
      installing.value = false
    }
  }

  function dismissInstall () {
    installDismissed.value = true
  }

  // Handle update
  async function handleUpdate () {
    updating.value = true
    try {
      await updateServiceWorker(true)
    } finally {
      updating.value = false
    }
  }

  function dismissUpdate () {
    updateDismissed.value = true
  }

  // Watch for offline ready state
  watch(offlineReady, ready => {
    if (ready) {
      showOfflineReady.value = true
    }
  })
</script>

<style scoped>
    .v-banner {
        position: sticky;
        top: 0;
        z-index: 1000;
    }
</style>
