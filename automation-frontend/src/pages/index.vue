<template>
  {{ temperatureData }}
</template>

<script lang="ts" setup>
import {supabase} from '../utils/supabase'

const temperatureData = ref([])
const humidityData = ref([])

onMounted(async () => {
  const {
    data: dbTemperatureData,
    error
  } = await supabase.from('environment_measurements').select().eq('type', 'temperature')
  temperatureData.value = dbTemperatureData;

  const {
    data: dbHumidityData,
    error: humidityError
  } = await supabase.from('environment_measurements').select().eq('type', 'humidity')
})
humidityData.value = dbHumidityData;

</script>
