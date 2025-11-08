<template>
  <v-app>
    <PWAPrompt />
    <router-view />
  </v-app>
</template>

<script lang="ts" setup>
  import { onMounted } from 'vue'
  import { usePWA } from '@/composables/usePWA'

  const { initPWA } = usePWA()

  onMounted(() => {
    initPWA()
  })
</script>
