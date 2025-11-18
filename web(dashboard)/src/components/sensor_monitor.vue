<script setup lang="ts">
import type { SensorReading, ThresholdConfig } from '@/lib/supabase'

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
  return new Date(date).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
}

const getSensorColor = (sensorNum: number) => {
  const colors = {
    1: { primary: '#42b883', bg: 'rgba(66, 184, 131, 0.1)' },
    2: { primary: '#4fc08d', bg: 'rgba(79, 192, 141, 0.1)' },
    3: { primary: '#35495e', bg: 'rgba(53, 73, 94, 0.1)' }
  }
  return colors[sensorNum as keyof typeof colors]
}
</script>

<template>
  <div class="card">
    <div class="card-header">
      <h2 class="card-title">Sensor Readings</h2>
      <div v-if="reading" class="timestamp">{{ formatTime(reading.created_at) }}</div>
    </div>
    
    <div v-if="!reading" class="no-data">
      <div class="no-data-icon">◇</div>
      <div class="no-data-text">No data available</div>
    </div>

    <div v-else class="sensors-grid">
      <!-- sensor 1 -->
      <div class="sensor-card" :style="{ '--sensor-color': getSensorColor(1).primary, '--sensor-bg': getSensorColor(1).bg }">
        <div class="sensor-header">
          <div class="sensor-label">
            <span class="sensor-number">01</span>
            <span class="sensor-name">Temperature</span>
          </div>
          <div 
            class="status-indicator"
            :class="checkThreshold(reading.temperature, 'temperature') ? 'active' : 'inactive'"
          >
            <span class="status-dot"></span>
          </div>
        </div>
        
        <div class="sensor-value-container">
          <div class="sensor-value">{{ reading.temperature.toFixed(1) }}</div>
          <div class="sensor-unit">units</div>
        </div>
        
        <div class="sensor-threshold" v-if="getThresholdForSensor('temperature')">
          <span class="threshold-label">Threshold</span>
          <span class="threshold-value">
            {{ getThresholdForSensor('temperature')?.comparison_operator }} 
            {{ getThresholdForSensor('temperature')?.threshold_value }}
          </span>
        </div>
      </div>

      <!-- sensor 2 -->
      <div class="sensor-card" :style="{ '--sensor-color': getSensorColor(2).primary, '--sensor-bg': getSensorColor(2).bg }">
        <div class="sensor-header">
          <div class="sensor-label">
            <span class="sensor-number">02</span>
            <span class="sensor-name">Humidity</span>
          </div>
          <div 
            class="status-indicator"
            :class="checkThreshold(reading.humidity, 'humidity') ? 'active' : 'inactive'"
          >
            <span class="status-dot"></span>
          </div>
        </div>
        
        <div class="sensor-value-container">
          <div class="sensor-value">{{ reading.humidity.toFixed(1) }}</div>
          <div class="sensor-unit">units</div>
        </div>
        
        <div class="sensor-threshold" v-if="getThresholdForSensor('humidity')">
          <span class="threshold-label">Threshold</span>
          <span class="threshold-value">
            {{ getThresholdForSensor('humidity')?.comparison_operator }} 
            {{ getThresholdForSensor('humidity')?.threshold_value }}
          </span>
        </div>
      </div>

      <!-- sensor 3 -->
      <div class="sensor-card" v-if="reading.gas !== null" :style="{ '--sensor-color': getSensorColor(3).primary, '--sensor-bg': getSensorColor(3).bg }">
        <div class="sensor-header">
          <div class="sensor-label">
            <span class="sensor-number">03</span>
            <span class="sensor-name">Gas</span>
          </div>
          <div 
            class="status-indicator"
            :class="checkThreshold(reading.gas, 'gas') ? 'active' : 'inactive'"
          >
            <span class="status-dot"></span>
          </div>
        </div>
        
        <div class="sensor-value-container">
          <div class="sensor-value">{{ reading.gas.toFixed(1) }}</div>
          <div class="sensor-unit">units</div>
        </div>
        
        <div class="sensor-threshold" v-if="getThresholdForSensor('gas')">
          <span class="threshold-label">Threshold</span>
          <span class="threshold-value">
            {{ getThresholdForSensor('gas')?.comparison_operator }} 
            {{ getThresholdForSensor('gas')?.threshold_value }}
          </span>
        </div>
      </div>
    </div>
  </div>
</template>

<style scoped>
.card {
  background: var(--bg-card);
  border-radius: 8px;
  padding: 0.65rem;
  box-shadow: 0 2px 6px rgba(0, 0, 0, 0.1);
  border: 1px solid var(--border-color);
  height: 100%;
  display: flex;
  flex-direction: column;
  position: relative;
  overflow: hidden;
}

.card::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  right: 0;
  height: 2px;
  background: linear-gradient(90deg, #42b883 0%, #4fc08d 50%, #35495e 100%);
  opacity: 0.6;
}

.card-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 0.6rem;
}

.card-title {
  margin: 0;
  font-size: 0.8rem;
  font-weight: 600;
  color: var(--text-primary);
  letter-spacing: -0.01em;
}

.timestamp {
  font-size: 0.65rem;
  color: var(--text-muted);
  font-weight: 500;
  padding: 0.2rem 0.4rem;
  background: var(--bg-secondary);
  border-radius: 4px;
}

.no-data {
  flex: 1;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: 0.4rem;
  opacity: 0.5;
}

.no-data-icon {
  font-size: 1.5rem;
  color: var(--text-muted);
}

.no-data-text {
  font-size: 0.7rem;
  color: var(--text-muted);
}

.sensors-grid {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 0.5rem;
  flex: 1;
}

.sensor-card {
  background: var(--sensor-bg);
  border: 1px solid rgba(255, 255, 255, 0.05);
  border-radius: 6px;
  padding: 0.5rem;
  display: flex;
  flex-direction: column;
  position: relative;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
  overflow: hidden;
}

.sensor-card::before {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  width: 2px;
  height: 100%;
  background: var(--sensor-color);
  opacity: 0.8;
  transition: opacity 0.3s;
}

.sensor-card:hover {
  transform: translateY(-1px);
  box-shadow: 0 3px 8px rgba(0, 0, 0, 0.15);
  border-color: var(--sensor-color);
}

.sensor-card:hover::before {
  opacity: 1;
}

.sensor-header {
  display: flex;
  justify-content: space-between;
  align-items: flex-start;
  margin-bottom: 0.4rem;
}

.sensor-label {
  display: flex;
  flex-direction: column;
  gap: 0.05rem;
}

.sensor-number {
  font-size: 0.75rem;
  font-weight: 700;
  color: var(--sensor-color);
  line-height: 1;
  letter-spacing: -0.02em;
}

.sensor-name {
  font-size: 0.6rem;
  color: var(--text-muted);
  text-transform: uppercase;
  letter-spacing: 0.3px;
  font-weight: 500;
}

.status-indicator {
  display: flex;
  align-items: center;
  justify-content: center;
  width: 20px;
  height: 20px;
  border-radius: 50%;
  background: var(--bg-secondary);
  border: 1.5px solid var(--border-color);
  transition: all 0.3s;
}

.status-indicator.active {
  background: var(--success-bg);
  border-color: var(--success);
  box-shadow: 0 0 0 2px rgba(66, 184, 131, 0.15);
}

.status-dot {
  width: 6px;
  height: 6px;
  border-radius: 50%;
  background: var(--text-muted);
  transition: all 0.3s;
}

.status-indicator.active .status-dot {
  background: var(--success);
  box-shadow: 0 0 6px var(--success);
  animation: pulse-dot 2s ease-in-out infinite;
}

@keyframes pulse-dot {
  0%, 100% { transform: scale(1); opacity: 1; }
  50% { transform: scale(1.2); opacity: 0.8; }
}

.sensor-value-container {
  flex: 1;
  display: flex;
  align-items: baseline;
  gap: 0.25rem;
  margin: 0.2rem 0;
}

.sensor-value {
  font-size: 1.5rem;
  font-weight: 700;
  color: var(--sensor-color);
  line-height: 1;
  letter-spacing: -0.03em;
  text-shadow: 0 2px 6px var(--sensor-bg);
}

.sensor-unit {
  font-size: 0.6rem;
  color: var(--text-muted);
  font-weight: 500;
  opacity: 0.7;
}

.sensor-threshold {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 0.3rem 0.4rem;
  background: rgba(0, 0, 0, 0.2);
  border-radius: 4px;
  margin-top: auto;
}

.threshold-label {
  font-size: 0.6rem;
  color: var(--text-muted);
  font-weight: 500;
  text-transform: uppercase;
  letter-spacing: 0.2px;
}

.threshold-value {
  font-size: 0.65rem;
  color: var(--sensor-color);
  font-weight: 600;
  font-family: 'SF Mono', 'Monaco', 'Consolas', monospace;
}

@media (max-width: 1200px) {
  .sensors-grid {
    grid-template-columns: repeat(2, 1fr);
  }
}

@media (max-width: 768px) {
  .sensors-grid {
    grid-template-columns: 1fr;
  }
}
</style>