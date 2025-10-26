import { createClient } from '@supabase/supabase-js'

const supabaseUrl = import.meta.env.VITE_SUPABASE_URL
const supabaseAnonKey = import.meta.env.VITE_SUPABASE_ANON_KEY

if (!supabaseUrl || !supabaseAnonKey) {
  throw new Error('Missing Supabase environment variables')
}

export const supabase = createClient(supabaseUrl, supabaseAnonKey)

// typeScript types for your database tables
export interface SensorReading {
  id: number
  created_at: string
  sensor_1_value: number
  sensor_2_value: number
  sensor_3_value: number | null
  all_thresholds_met: boolean
  notes: string | null
}

export interface ActuatorState {
  id: number
  created_at: string
  command: 'stop' | 'move'
  triggered_by_reading_id: number | null
  executed_at: string | null
  notes: string | null
}

export interface ThresholdConfig {
  id: number
  sensor_name: string
  threshold_value: number
  comparison_operator: '>' | '<' | '>=' | '<=' | '='
  is_active: boolean
  updated_at: string
}

export interface SystemLog {
  id: number
  created_at: string
  log_level: 'info' | 'warning' | 'error'
  source: string
  message: string
}

// database type helper
export interface Database {
  public: {
    Tables: {
      sensor_readings: {
        Row: SensorReading
        Insert: Omit<SensorReading, 'id' | 'created_at'>
        Update: Partial<Omit<SensorReading, 'id' | 'created_at'>>
      }
      actuator_states: {
        Row: ActuatorState
        Insert: Omit<ActuatorState, 'id' | 'created_at'>
        Update: Partial<Omit<ActuatorState, 'id' | 'created_at'>>
      }
      threshold_config: {
        Row: ThresholdConfig
        Insert: Omit<ThresholdConfig, 'id' | 'updated_at'>
        Update: Partial<Omit<ThresholdConfig, 'id' | 'updated_at'>>
      }
      system_logs: {
        Row: SystemLog
        Insert: Omit<SystemLog, 'id' | 'created_at'>
        Update: Partial<Omit<SystemLog, 'id' | 'created_at'>>
      }
    }
  }
}