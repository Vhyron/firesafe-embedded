<script setup lang="ts">
import { ref, onMounted, onUnmounted, computed } from 'vue'
import { Line } from 'vue-chartjs'
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  PointElement,
  LineElement,
  Title,
  Tooltip,
  Legend,
  Filler
} from 'chart.js'
import { supabase } from '@/lib/supabase'
import type { SensorReading } from '@/lib/supabase'
import '../style.css'

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

const readings = ref<SensorReading[]>([])
const timeRange = ref<'1h' | '6h' | '24h'>('1h')

const chartData = computed(() => {
  const labels = readings.value.map(r => 
    new Date(r.created_at).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
  )

  return {
    labels,
    datasets: [
      {
        label: 'Sensor 1',
        data: readings.value.map(r => r.sensor_1_value),
        borderColor: '#42b883',
        backgroundColor: 'rgba(66, 184, 131, 0.1)',
        tension: 0.4,
        fill: true,
        pointRadius: 3,
        pointHoverRadius: 6
      },
      {
        label: 'Sensor 2',
        data: readings.value.map(r => r.sensor_2_value),
        borderColor: '#35495e',
        backgroundColor: 'rgba(53, 73, 94, 0.1)',
        tension: 0.4,
        fill: true,
        pointRadius: 3,
        pointHoverRadius: 6
      },
      {
        label: 'Sensor 3',
        data: readings.value.map(r => r.sensor_3_value ?? 0),
        borderColor: '#4fc08d',
        backgroundColor: 'rgba(79, 192, 141, 0.1)',
        tension: 0.4,
        fill: true,
        pointRadius: 3,
        pointHoverRadius: 6
      }
    ]
  }
})

const chartOptions = computed(() => ({
  responsive: true,
  maintainAspectRatio: false,
  plugins: {
    legend: {
      display: true,
      position: 'top' as const,
      labels: {
        color: '#cbd5e1',
        font: {
          size: 12
        },
        padding: 15
      }
    },
    tooltip: {
      backgroundColor: '#1e293b',
      titleColor: '#f1f5f9',
      bodyColor: '#cbd5e1',
      borderColor: '#42b883',
      borderWidth: 1,
      padding: 12,
      displayColors: true
    }
  },
  scales: {
    x: {
      grid: {
        color: '#334155',
        drawBorder: false
      },
      ticks: {
        color: '#94a3b8',
        maxRotation: 45,
        minRotation: 45
      }
    },
    y: {
      grid: {
        color: '#334155',
        drawBorder: false
      },
      ticks: {
        color: '#94a3b8'
      }
    }
  }
}))

const fetchReadings = async () => {
  const hoursAgo = timeRange.value === '1h' ? 1 : timeRange.value === '6h' ? 6 : 24
  const cutoffTime = new Date()
  cutoffTime.setHours(cutoffTime.getHours() - hoursAgo)

  const { data, error } = await supabase
    .from('sensor_readings')
    .select('*')
    .gte('created_at', cutoffTime.toISOString())
    .order('created_at', { ascending: true })

  if (error) {
    console.error('Error fetching readings:', error)
    return
  }

  readings.value = data || []
}

let subscription: any
let refreshInterval: any

onMounted(() => {
  fetchReadings()

  // refresh every 5 seconds
  refreshInterval = setInterval(fetchReadings, 5000)

  // realtime subscription
  subscription = supabase
    .channel('chart_readings')
    .on(
      'postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'sensor_readings' },
      () => {
        fetchReadings()
      }
    )
    .subscribe()
})

onUnmounted(() => {
  if (subscription) supabase.removeChannel(subscription)
  if (refreshInterval) clearInterval(refreshInterval)
})

const changeTimeRange = (range: '1h' | '6h' | '24h') => {
  timeRange.value = range
  fetchReadings()
}
</script>

<template>
  <div class="card">
    <div class="chart-header">
      <h2 class="card-title">Sensor Trends</h2>
      <div class="time-range-buttons">
        <button 
          @click="changeTimeRange('1h')"
          :class="{ active: timeRange === '1h' }"
        >
          1H
        </button>
        <button 
          @click="changeTimeRange('6h')"
          :class="{ active: timeRange === '6h' }"
        >
          6H
        </button>
        <button 
          @click="changeTimeRange('24h')"
          :class="{ active: timeRange === '24h' }"
        >
          24H
        </button>
      </div>
    </div>

    <div class="chart-container">
      <Line v-if="readings.length > 0" :data="chartData" :options="chartOptions" />
      <div v-else class="no-data">
        No data available for the selected time range
      </div>
    </div>
  </div>
</template>

<style scoped>
.card {
  background: var(--bg-card);
  border-radius: 12px;
  padding: 1.5rem;
  box-shadow: var(--shadow);
  border: 1px solid var(--border-color);
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 1.5rem;
  flex-wrap: wrap;
  gap: 1rem;
}

.card-title {
  margin: 0;
  font-size: 1.25rem;
  font-weight: 600;
  color: var(--text-primary);
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.card-title::before {
  content: '📈';
  font-size: 1.5rem;
}

.time-range-buttons {
  display: flex;
  gap: 0.5rem;
}

.time-range-buttons button {
  padding: 0.5rem 1rem;
  background: var(--bg-secondary);
  color: var(--text-secondary);
  border: 1px solid var(--border-color);
  border-radius: 6px;
  cursor: pointer;
  font-weight: 600;
  transition: all 0.2s;
}

.time-range-buttons button:hover {
  background: var(--bg-hover);
  color: var(--text-primary);
}

.time-range-buttons button.active {
  background: var(--accent-primary);
  color: white;
  border-color: var(--accent-primary);
}

.chart-container {
  height: 400px;
  position: relative;
}

.no-data {
  display: flex;
  align-items: center;
  justify-content: center;
  height: 100%;
  color: var(--text-muted);
  font-size: 1.1rem;
}

@media (max-width: 768px) {
  .chart-container {
    height: 300px;
  }
}
</style>