<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { supabase } from '@/lib/supabase'
import type { SensorReading, ActuatorState, ThresholdConfig } from '@/lib/supabase'

import SensorMonitor from '@/components/sensor_monitor.vue'
import ActuatorStatus from '@/components/actuator_status.vue'
import SensorChart from '@/components/sensor_chart.vue'
import ThresholdConfigg from '@/components/threshold_config.vue'
import RecentActivity from '@/components/recent_activity.vue'

const latestReading = ref<SensorReading | null>(null)
const latestActuator = ref<ActuatorState | null>(null)
const thresholds = ref<ThresholdConfig[]>([])
const isLoading = ref(true)

let readingsSubscription: any
let actuatorSubscription: any

const fetchInitialData = async () => {
  try {
    const { data: readingData } = await supabase
      .from('sensor_readings')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(1)
      .single()
    
    if (readingData) latestReading.value = readingData

    const { data: actuatorData } = await supabase
      .from('actuator_states')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(1)
      .single()
    
    if (actuatorData) latestActuator.value = actuatorData

    const { data: thresholdData } = await supabase
      .from('threshold_config')
      .select('*')
      .order('sensor_name')
    
    if (thresholdData) thresholds.value = thresholdData

    isLoading.value = false
  } catch (error) {
    console.error('Error fetching data:', error)
    isLoading.value = false
  }
}

const setupRealtimeSubscriptions = () => {
  readingsSubscription = supabase
    .channel('sensor_readings_changes')
    .on(
      'postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'sensor_readings' },
      (payload) => {
        latestReading.value = payload.new as SensorReading
      }
    )
    .subscribe()

  actuatorSubscription = supabase
    .channel('actuator_states_changes')
    .on(
      'postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'actuator_states' },
      (payload) => {
        latestActuator.value = payload.new as ActuatorState
      }
    )
    .subscribe()
}

onMounted(() => {
  fetchInitialData()
  setupRealtimeSubscriptions()
})

onUnmounted(() => {
  if (readingsSubscription) supabase.removeChannel(readingsSubscription)
  if (actuatorSubscription) supabase.removeChannel(actuatorSubscription)
})
</script>

<template>
  <div class="dashboard">
    <header class="header">
      <div class="container">
        <h1>FireSafe Dashboard</h1>
        <p class="subtitle">Real-time Environmental Monitoring</p>
      </div>
    </header>

    <div v-if="isLoading" class="container">
      <div class="loading">Loading...</div>
    </div>

    <div v-else class="container">
      <!-- top row: sensors & actuator -->
      <div class="grid-2">
        <SensorMonitor 
          :reading="latestReading" 
          :thresholds="thresholds"
        />
        <ActuatorStatus 
          :state="latestActuator"
          :thresholds-met="latestReading?.all_thresholds_met ?? false"
        />
      </div>

      <!-- middle row: chart -->
      <div class="chart-section">
        <SensorChart />
      </div>

      <!-- bottom row: config & activity -->
      <div class="grid-2">
        <ThresholdConfigg 
          :thresholds="thresholds" 
          @updated="fetchInitialData"
        />
        <RecentActivity />
      </div>
    </div>
  </div>
</template>

<style scoped>
.dashboard {
  min-height: 100vh;
  background: var(--bg-primary);
}

.header {
  background: linear-gradient(135deg, #42b883 0%, #35495e 100%);
  color: white;
  padding: 1.25rem 0;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.4);
}

.container {
  max-width: 1400px;
  margin: 0 auto;
  padding: 0 1rem;
}

.header h1 {
  margin: 0;
  font-size: 1.5rem;
  font-weight: 700;
}

.subtitle {
  margin: 0.25rem 0 0 0;
  opacity: 0.9;
  font-size: 0.85rem;
}

.loading {
  text-align: center;
  padding: 3rem;
  font-size: 1rem;
  color: var(--text-muted);
}

.grid-2 {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 1rem;
  margin-top: 1rem;
}

.chart-section {
  margin-top: 1rem;
}

@media (max-width: 768px) {
  .header h1 {
    font-size: 1.25rem;
  }
  
  .grid-2 {
    grid-template-columns: 1fr;
  }
}
</style>