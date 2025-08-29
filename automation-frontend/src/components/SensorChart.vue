<template>
  <div class="sensor-chart" :style="{ height: `${height}px` }">
    <Line
      ref="chartRef"
      :data="chartData"
      :options="chartOptions"
    />
  </div>
</template>

<script lang="ts" setup>
import { ref, computed, watch, type PropType } from 'vue'
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler,
  type ChartOptions,
  type ChartData
} from 'chart.js'
import { Line } from 'vue-chartjs'
import type { EnvironmentMeasurement } from '@/types/sensor'

// Register Chart.js components
ChartJS.register(
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
)

interface Props {
  data: EnvironmentMeasurement[]
  type: 'temperature' | 'humidity'
  height?: number
  title?: string
}

const props = withDefaults(defineProps<Props>(), {
  height: 300,
  title: ''
})

const chartRef = ref()

const chartData = computed<ChartData<'line'>>(() => {
  const sortedData = [...props.data].sort(
    (a, b) => new Date(a.created_at).getTime() - new Date(b.created_at).getTime()
  )
  
  const labels = sortedData.map(item => {
    const date = new Date(item.created_at)
    return date.toLocaleString('de-DE', {
      month: 'short',
      day: '2-digit',
      hour: '2-digit',
      minute: '2-digit'
    })
  })
  
  const values = sortedData.map(item => item.value)
  
  const color = props.type === 'temperature' ? '#1976D2' : '#388E3C'
  const bgColor = props.type === 'temperature' ? 'rgba(25, 118, 210, 0.1)' : 'rgba(56, 142, 60, 0.1)'
  
  return {
    labels,
    datasets: [
      {
        label: props.type === 'temperature' ? 'Temperatur (°C)' : 'Luftfeuchtigkeit (%)',
        data: values,
        borderColor: color,
        backgroundColor: bgColor,
        borderWidth: 3,
        pointRadius: 4,
        pointHoverRadius: 6,
        pointBackgroundColor: color,
        pointBorderColor: '#ffffff',
        pointBorderWidth: 2,
        fill: true,
        tension: 0.3
      }
    ]
  }
})

const chartOptions = computed<ChartOptions<'line'>>(() => ({
  responsive: true,
  maintainAspectRatio: false,
  interaction: {
    intersect: false,
    mode: 'index'
  },
  plugins: {
    title: {
      display: !!props.title,
      text: props.title,
      font: {
        size: 18,
        weight: 'bold'
      },
      color: '#1976D2',
      padding: 20
    },
    legend: {
      display: false
    },
    tooltip: {
      backgroundColor: 'rgba(255, 255, 255, 0.95)',
      titleColor: '#1976D2',
      bodyColor: '#424242',
      borderColor: '#1976D2',
      borderWidth: 1,
      cornerRadius: 8,
      padding: 12,
      titleFont: {
        size: 14,
        weight: 'bold'
      },
      bodyFont: {
        size: 14
      },
      callbacks: {
        label: (context) => {
          const value = context.parsed.y
          const unit = props.type === 'temperature' ? '°C' : '%'
          return `${value.toFixed(1)}${unit}`
        }
      }
    }
  },
  scales: {
    x: {
      display: true,
      grid: {
        color: 'rgba(0, 0, 0, 0.1)',
        drawOnChartArea: true
      },
      ticks: {
        font: {
          size: 12
        },
        color: '#666666',
        maxRotation: 45,
        minRotation: 0
      },
      title: {
        display: true,
        text: 'Zeit',
        font: {
          size: 14,
          weight: 'bold'
        },
        color: '#1976D2'
      }
    },
    y: {
      display: true,
      grid: {
        color: 'rgba(0, 0, 0, 0.1)',
        drawOnChartArea: true
      },
      ticks: {
        font: {
          size: 12
        },
        color: '#666666',
        callback: (value) => {
          const unit = props.type === 'temperature' ? '°C' : '%'
          return `${value}${unit}`
        }
      },
      title: {
        display: true,
        text: props.type === 'temperature' ? 'Temperatur (°C)' : 'Luftfeuchtigkeit (%)',
        font: {
          size: 14,
          weight: 'bold'
        },
        color: '#1976D2'
      }
    }
  },
  elements: {
    point: {
      hoverBackgroundColor: '#1976D2'
    }
  }
}))

// Watch for data changes and update chart
watch(
  () => props.data,
  () => {
    if (chartRef.value?.chart) {
      chartRef.value.chart.update('none')
    }
  },
  { deep: true }
)
</script>

<style scoped>
.sensor-chart {
  width: 100%;
  position: relative;
}
</style>