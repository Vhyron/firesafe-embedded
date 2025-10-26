<script setup lang="ts">
import { computed } from 'vue'
import type { ActuatorState } from '@/lib/supabase'
import '../style.css'

const props = defineProps<{
  state: ActuatorState | null
  thresholdsMet: boolean
}>()

const isMoving = computed(() => props.state?.command === 'move')

const statusText = computed(() => {
  if (!props.state) return 'Unknown'
  return isMoving.value ? 'Moving (180°)' : 'Stopped'
})

const statusColor = computed(() => {
  if (!props.state) return 'var(--text-muted)'
  return isMoving.value ? 'var(--accent-primary)' : 'var(--error)'
})

const formatTime = (date: string) => {
  return new Date(date).toLocaleTimeString()
}
</script>

<template>
  <div class="card">
    <h2 class="card-title">Servo Actuator</h2>
    
    <div v-if="!state" class="no-data">
      No actuator data available
    </div>

    <div v-else class="actuator-content">
      <!-- status indicator -->
      <div class="status-circle" :class="{ moving: isMoving, stopped: !isMoving }">
        <div class="pulse" v-if="isMoving"></div>
        <div class="icon">
          {{ isMoving ? '↻' : '■' }}
        </div>
      </div>

      <!-- status text -->
      <div class="status-text" :style="{ color: statusColor }">
        {{ statusText }}
      </div>

      <!-- threshold status -->
      <div class="threshold-status">
        <div class="threshold-label">All Thresholds:</div>
        <div class="threshold-badge" :class="{ met: thresholdsMet, 'not-met': !thresholdsMet }">
          {{ thresholdsMet ? '✓ Met' : '✗ Not Met' }}
        </div>
      </div>

      <!-- info box -->
      <div class="info-box">
        <div class="info-item">
          <span class="info-label">Current Command:</span>
          <span class="info-value">{{ state.command.toUpperCase() }}</span>
        </div>
        <div class="info-item">
          <span class="info-label">Last Update:</span>
          <span class="info-value">{{ formatTime(state.created_at) }}</span>
        </div>
      </div>

      <!-- behavior explanation -->
      <div class="explanation">
        <strong>Behavior:</strong> Servo continuously rotates at 180° when thresholds are not met. 
        Stops when all sensor thresholds are satisfied.
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
  content: '⚙️';
  font-size: 1.5rem;
}

.no-data {
  text-align: center;
  padding: 2rem;
  color: var(--text-muted);
}

.actuator-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1.5rem;
}

.status-circle {
  position: relative;
  width: 120px;
  height: 120px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  margin: 1rem 0;
}

.status-circle.moving {
  background: var(--success-bg);
  border: 3px solid var(--success);
  animation: rotate 2s linear infinite;
}

.status-circle.stopped {
  background: var(--error-bg);
  border: 3px solid var(--error);
}

.pulse {
  position: absolute;
  width: 100%;
  height: 100%;
  border-radius: 50%;
  background: var(--success);
  opacity: 0.3;
  animation: pulse 2s ease-out infinite;
}

@keyframes pulse {
  0% {
    transform: scale(1);
    opacity: 0.3;
  }
  100% {
    transform: scale(1.3);
    opacity: 0;
  }
}

@keyframes rotate {
  from {
    transform: rotate(0deg);
  }
  to {
    transform: rotate(360deg);
  }
}

.icon {
  font-size: 3rem;
  z-index: 1;
}

.status-text {
  font-size: 1.5rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 1px;
}

.threshold-status {
  display: flex;
  align-items: center;
  gap: 1rem;
  padding: 1rem;
  background: var(--bg-secondary);
  border-radius: 8px;
  width: 100%;
}

.threshold-label {
  font-weight: 600;
  color: var(--text-secondary);
}

.threshold-badge {
  padding: 0.5rem 1rem;
  border-radius: 8px;
  font-weight: 600;
  margin-left: auto;
}

.threshold-badge.met {
  background: var(--success-bg);
  color: var(--success);
  border: 1px solid var(--success);
}

.threshold-badge.not-met {
  background: var(--error-bg);
  color: var(--error);
  border: 1px solid var(--error);
}

.info-box {
  width: 100%;
  background: var(--bg-secondary);
  border-radius: 8px;
  padding: 1rem;
  display: flex;
  flex-direction: column;
  gap: 0.75rem;
}

.info-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.info-label {
  color: var(--text-muted);
  font-size: 0.9rem;
}

.info-value {
  color: var(--accent-primary);
  font-weight: 600;
}

.explanation {
  width: 100%;
  padding: 1rem;
  background: var(--bg-hover);
  border-left: 3px solid var(--accent-primary);
  border-radius: 4px;
  font-size: 0.85rem;
  color: var(--text-secondary);
  line-height: 1.5;
}

.explanation strong {
  color: var(--accent-primary);
}
</style>