<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { supabase } from '@/lib/supabase'
import type { SensorReading, ActuatorState } from '@/lib/supabase'
import { formatDistanceToNow } from 'date-fns'

interface ActivityItem {
  id: number
  type: 'sensor' | 'actuator'
  timestamp: string
  data: SensorReading | ActuatorState
}

const activities = ref<ActivityItem[]>([])
const isLoading = ref(true)

const fetchRecentActivity = async () => {
  try {
    // fetch recent sensor readings
    const { data: readings } = await supabase
      .from('sensor_readings')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(5)

    // fetch recent actuator states
    const { data: states } = await supabase
      .from('actuator_states')
      .select('*')
      .order('created_at', { ascending: false })
      .limit(5)

    // combine and sort by timestamp
    const combined: ActivityItem[] = [
      ...(readings || []).map(r => ({
        id: r.id,
        type: 'sensor' as const,
        timestamp: r.created_at,
        data: r
      })),
      ...(states || []).map(s => ({
        id: s.id,
        type: 'actuator' as const,
        timestamp: s.created_at,
        data: s
      }))
    ]

    activities.value = combined
      .sort((a, b) => new Date(b.timestamp).getTime() - new Date(a.timestamp).getTime())
      .slice(0, 10)

    isLoading.value = false
  } catch (error) {
    console.error('Error fetching activity:', error)
    isLoading.value = false
  }
}

const formatTime = (date: string) => {
  return formatDistanceToNow(new Date(date), { addSuffix: true })
}

const getActivityIcon = (type: string) => {
  return type === 'sensor' ? '📊' : '⚙️'
}

const getActivityColor = (item: ActivityItem) => {
  if (item.type === 'sensor') {
    return (item.data as SensorReading).all_thresholds_met ? 'var(--success)' : 'var(--text-muted)'
  } else {
    return (item.data as ActuatorState).command === 'stop' ? 'var(--error)' : 'var(--success)'
  }
}

const getActivityDescription = (item: ActivityItem) => {
  if (item.type === 'sensor') {
    const reading = item.data as SensorReading
    return `Sensor reading: S1=${reading.sensor_1_value.toFixed(1)}, S2=${reading.sensor_2_value.toFixed(1)}, S3=${reading.sensor_3_value?.toFixed(1) ?? 'N/A'}`
  } else {
    const state = item.data as ActuatorState
    return `Actuator ${state.command === 'stop' ? 'stopped' : 'started moving'}`
  }
}

let subscription: any
let refreshInterval: any

onMounted(() => {
  fetchRecentActivity()

  // refresh every 5 seconds
  refreshInterval = setInterval(fetchRecentActivity, 5000)

  // realtime subscriptions
  subscription = supabase
    .channel('activity_changes')
    .on(
      'postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'sensor_readings' },
      () => fetchRecentActivity()
    )
    .on(
      'postgres_changes',
      { event: 'INSERT', schema: 'public', table: 'actuator_states' },
      () => fetchRecentActivity()
    )
    .subscribe()
})

onUnmounted(() => {
  if (subscription) supabase.removeChannel(subscription)
  if (refreshInterval) clearInterval(refreshInterval)
})
</script>

<template>
  <div class="card">
    <h2 class="card-title">Recent Activity</h2>

    <div v-if="isLoading" class="loading">
      Loading activity...
    </div>

    <div v-else-if="activities.length === 0" class="no-data">
      No recent activity
    </div>

    <div v-else class="activity-list">
      <div 
        v-for="item in activities" 
        :key="`${item.type}-${item.id}`"
        class="activity-item"
      >
        <div class="activity-icon" :style="{ color: getActivityColor(item) }">
          {{ getActivityIcon(item.type) }}
        </div>
        
        <div class="activity-content">
          <div class="activity-description">
            {{ getActivityDescription(item) }}
          </div>
          <div class="activity-time">
            {{ formatTime(item.timestamp) }}
          </div>
        </div>

        <div 
          v-if="item.type === 'sensor'"
          class="status-indicator"
          :class="{ met: (item.data as SensorReading).all_thresholds_met }"
        >
          {{ (item.data as SensorReading).all_thresholds_met ? '✓' : '✗' }}
        </div>
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
  max-height: 600px;
  display: flex;
  flex-direction: column;
}

.card-title {
  margin: 0 0 1.5rem 0;
  font-size: 1.25rem;
  font-weight: 600;
  color: var(--text-primary);
  display: flex;
  align-items: center;
  gap: 0.5rem;
}

.card-title::before {
  content: '📋';
  font-size: 1.5rem;
}

.loading,
.no-data {
  text-align: center;
  padding: 2rem;
  color: var(--text-muted);
}

.activity-list {
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
  overflow-y: auto;
  padding-right: 0.5rem;
}

/* Custom Scrollbar */
.activity-list::-webkit-scrollbar {
  width: 6px;
}

.activity-list::-webkit-scrollbar-track {
  background: var(--bg-secondary);
  border-radius: 3px;
}

.activity-list::-webkit-scrollbar-thumb {
  background: var(--accent-primary);
  border-radius: 3px;
}

.activity-list::-webkit-scrollbar-thumb:hover {
  background: var(--accent-light);
}

.activity-item {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: var(--bg-secondary);
  border-radius: 8px;
  transition: all 0.2s;
  border-left: 3px solid transparent;
}

.activity-item:hover {
  background: var(--bg-hover);
  border-left-color: var(--accent-primary);
  transform: translateX(4px);
}

.activity-icon {
  font-size: 1.5rem;
  flex-shrink: 0;
}

.activity-content {
  flex: 1;
  min-width: 0;
}

.activity-description {
  color: var(--text-primary);
  font-size: 0.9rem;
  margin-bottom: 0.25rem;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.activity-time {
  color: var(--text-muted);
  font-size: 0.8rem;
}

.status-indicator {
  flex-shrink: 0;
  width: 28px;
  height: 28px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  font-weight: 700;
  font-size: 0.9rem;
}

.status-indicator.met {
  background: var(--success-bg);
  color: var(--success);
  border: 1px solid var(--success);
}

.status-indicator:not(.met) {
  background: var(--error-bg);
  color: var(--error);
  border: 1px solid var(--error);
}
</style>