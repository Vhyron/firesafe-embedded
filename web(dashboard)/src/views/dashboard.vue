<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { supabase } from '@/lib/supabase'
import type { SensorReading, ActuatorState, ThresholdConfig } from '@/lib/supabase'
import '../style.css'

import SensorMonitor from '@/components/sensor_monitor.vue'
import ActuatorStatus from '@/components/actuator_status.vue'
import SensorChart from '@/components/sensor_chart.vue'
import ThresholdConfigg from '@/components/threshold_config.vue'
import RecentActivity from '@/components/recent_activity.vue'

// state
const latestReading = ref<SensorReading | null>(null)
const latestActuator = ref<ActuatorState | null>(null)
const thresholds = ref<ThresholdConfig[]>([])
const isLoading = ref(true)

// realtime subscriptions
let readingsSubscription: any
let actuatorSubscription: any

const fetchInitialData = async () => {
  try {
    // get latest sensor reading
    const { data: readingData } = await supabase
      .from('sensor_readings')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(1)
      .single()
    
    if (readingData) latestReading.value = readingData

    // get latest actuator state
    const { data: actuatorData } = await supabase
      .from('actuator_states')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(1)
      .single()
    
    if (actuatorData) latestActuator.value = actuatorData

    // get thresholds
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
  // subscribe to sensor readings
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

  // subscribe to actuator states
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
        <p class="subtitle">Real-time Environmental Monitoring System</p>
      </div>
    </header>

    <div v-if="isLoading" class="container">
      <div class="loading">Loading dashboard...</div>
    </div>

    <!-- main dashboard -->
    <div v-else class="container">

      <!-- sensor monitor & actuator status -->
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

      <!-- chart section -->
      <div class="chart-section">
        <SensorChart />
      </div>

      <!-- threshold Config & recent activity -->
      <div class="grid-2">
        <ThresholdConfigg :thresholds="thresholds" />
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
  padding: 2rem 0;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.4);
}

.container {
  max-width: 1400px;
  margin: 0 auto;
  padding: 0 1rem;
}

.header h1 {
  margin: 0;
  font-size: 2rem;
  font-weight: 700;
}

.subtitle {
  margin: 0.5rem 0 0 0;
  opacity: 0.9;
  font-size: 1rem;
}

.loading {
  text-align: center;
  padding: 4rem;
  font-size: 1.2rem;
  color: var(--text-muted);
}

.grid-2 {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
  gap: 1.5rem;
  margin-top: 1.5rem;
}

.chart-section {
  margin-top: 1.5rem;
}

@media (max-width: 768px) {
  .header h1 {
    font-size: 1.5rem;
  }
  
  .grid-2 {
    grid-template-columns: 1fr;
  }
}
</style>