<script setup lang="ts">
import type { SensorReading, ThresholdConfig } from '@/lib/supabase'
import '../style.css'

const props = defineProps<{
  reading: SensorReading | null
  thresholds: ThresholdConfig[]
}>()

const getThresholdForSensor = (sensorName: string) => {
  return props.thresholds.find(t => t.sensor_name === sensorName)
}

const checkThreshold = (value: number, sensorName: string) => {
  const threshold = getThresholdForSensor(sensorName)
  if (!threshold) return false
  
  switch (threshold.comparison_operator) {
    case '>': return value > threshold.threshold_value
    case '<': return value < threshold.threshold_value
    case '>=': return value >= threshold.threshold_value
    case '<=': return value <= threshold.threshold_value
    case '=': return value === threshold.threshold_value
    default: return false
  }
}

const formatTime = (date: string) => {
  return new Date(date).toLocaleTimeString()
}
</script>

<template>
  <div class="card">
    <h2 class="card-title">Sensor Readings</h2>
    
    <div v-if="!reading" class="no-data">
      No sensor data available
    </div>

    <div v-else class="sensors">
      <div class="sensor-item">
        <div class="sensor-header">
          <span class="sensor-name">Sensor 1</span>
          <span 
            class="status-badge"
            :class="checkThreshold(reading.sensor_1_value, 'sensor_1') ? 'met' : 'not-met'"
          >
            {{ checkThreshold(reading.sensor_1_value, 'sensor_1') ? '✓' : '✗' }}
          </span>
        </div>
        <div class="sensor-value">{{ reading.sensor_1_value.toFixed(2) }}</div>
        <div class="sensor-threshold" v-if="getThresholdForSensor('sensor_1')">
          Threshold: {{ getThresholdForSensor('sensor_1')?.comparison_operator }} 
          {{ getThresholdForSensor('sensor_1')?.threshold_value }}
        </div>
      </div>

      <div class="sensor-item">
        <div class="sensor-header">
          <span class="sensor-name">Sensor 2</span>
          <span 
            class="status-badge"
            :class="checkThreshold(reading.sensor_2_value, 'sensor_2') ? 'met' : 'not-met'"
          >
            {{ checkThreshold(reading.sensor_2_value, 'sensor_2') ? '✓' : '✗' }}
          </span>
        </div>
        <div class="sensor-value">{{ reading.sensor_2_value.toFixed(2) }}</div>
        <div class="sensor-threshold" v-if="getThresholdForSensor('sensor_2')">
          Threshold: {{ getThresholdForSensor('sensor_2')?.comparison_operator }} 
          {{ getThresholdForSensor('sensor_2')?.threshold_value }}
        </div>
      </div>

      <div class="sensor-item" v-if="reading.sensor_3_value !== null">
        <div class="sensor-header">
          <span class="sensor-name">Sensor 3</span>
          <span 
            class="status-badge"
            :class="checkThreshold(reading.sensor_3_value, 'sensor_3') ? 'met' : 'not-met'"
          >
            {{ checkThreshold(reading.sensor_3_value, 'sensor_3') ? '✓' : '✗' }}
          </span>
        </div>
        <div class="sensor-value">{{ reading.sensor_3_value.toFixed(2) }}</div>
        <div class="sensor-threshold" v-if="getThresholdForSensor('sensor_3')">
          Threshold: {{ getThresholdForSensor('sensor_3')?.comparison_operator }} 
          {{ getThresholdForSensor('sensor_3')?.threshold_value }}
        </div>
      </div>

      <div class="last-updated">
        Last updated: {{ formatTime(reading.created_at) }}
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
  content: '📊';
  font-size: 1.5rem;
}

.no-data {
  text-align: center;
  padding: 2rem;
  color: var(--text-muted);
}

.sensors {
  display: flex;
  flex-direction: column;
  gap: 1rem;
}

.sensor-item {
  padding: 1rem;
  background: var(--bg-secondary);
  border-radius: 8px;
  border-left: 4px solid var(--accent-primary);
  transition: transform 0.2s;
}

.sensor-item:hover {
  transform: translateX(4px);
}

.sensor-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 0.5rem;
}

.sensor-name {
  font-weight: 600;
  color: var(--text-secondary);
  font-size: 0.9rem;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.status-badge {
  padding: 0.25rem 0.75rem;
  border-radius: 12px;
  font-size: 0.85rem;
  font-weight: 600;
}

.status-badge.met {
  background: var(--success-bg);
  color: var(--success);
  border: 1px solid var(--success);
}

.status-badge.not-met {
  background: var(--error-bg);
  color: var(--error);
  border: 1px solid var(--error);
}

.sensor-value {
  font-size: 2rem;
  font-weight: 700;
  color: var(--accent-primary);
  margin: 0.5rem 0;
  text-shadow: 0 0 20px rgba(66, 184, 131, 0.3);
}

.sensor-threshold {
  font-size: 0.85rem;
  color: var(--text-muted);
}

.last-updated {
  margin-top: 1rem;
  padding-top: 1rem;
  border-top: 1px solid var(--border-color);
  text-align: center;
  font-size: 0.85rem;
  color: var(--text-muted);
}
</style>