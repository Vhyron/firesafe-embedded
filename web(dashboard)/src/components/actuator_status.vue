<script setup lang="ts">
import { computed } from 'vue'
import type { ActuatorState } from '@/lib/supabase'

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
  return new Date(date).toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })
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
          {{ thresholdsMet ? '✓ Met' : '✕ Not Met' }}
        </div>
      </div>

      <!-- info box -->
      <div class="info-box">
        <div class="info-item">
          <span class="info-label">Command:</span>
          <span class="info-value">{{ state.command.toUpperCase() }}</span>
        </div>
        <div class="info-item">
          <span class="info-label">Updated:</span>
          <span class="info-value">{{ formatTime(state.created_at) }}</span>
        </div>
      </div>

      <!-- behavior explanation -->
      <div class="explanation">
        Servo rotates at 180° when thresholds not met. Stops when satisfied.
      </div>
    </div>
  </div>
</template>

<style scoped>
.card {
  background: var(--bg-card);
  border-radius: 10px;
  padding: 1rem;
  box-shadow: var(--shadow);
  border: 1px solid var(--border-color);
}

.card-title {
  margin: 0 0 0.85rem 0;
  font-size: 1rem;
  font-weight: 600;
  color: var(--text-primary);
}

.no-data {
  text-align: center;
  padding: 1.5rem;
  color: var(--text-muted);
  font-size: 0.85rem;
}

.actuator-content {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0.85rem;
}

.status-circle {
  position: relative;
  width: 90px;
  height: 90px;
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  margin: 0.5rem 0;
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
  font-size: 2.5rem;
  z-index: 1;
  line-height: 1;
}

.status-text {
  font-size: 1.25rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.5px;
}

.threshold-status {
  display: flex;
  align-items: center;
  gap: 0.75rem;
  padding: 0.75rem;
  background: var(--bg-secondary);
  border-radius: 8px;
  width: 100%;
}

.threshold-label {
  font-weight: 600;
  color: var(--text-secondary);
  font-size: 0.8rem;
}

.threshold-badge {
  padding: 0.35rem 0.75rem;
  border-radius: 8px;
  font-weight: 600;
  margin-left: auto;
  font-size: 0.75rem;
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
  padding: 0.75rem;
  display: flex;
  flex-direction: column;
  gap: 0.5rem;
}

.info-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.info-label {
  color: var(--text-muted);
  font-size: 0.75rem;
}

.info-value {
  color: var(--accent-primary);
  font-weight: 600;
  font-size: 0.8rem;
}

.explanation {
  width: 100%;
  padding: 0.75rem;
  background: var(--bg-hover);
  border-left: 3px solid var(--accent-primary);
  border-radius: 4px;
  font-size: 0.75rem;
  color: var(--text-secondary);
  line-height: 1.4;
}
</style>