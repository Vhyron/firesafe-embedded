<script setup lang="ts">
import { RouterView } from 'vue-router'
</script>

<template>
  <div id="app">
    <RouterView />
  </div>
</template>

<style scoped>
  @import url('https://fonts.googleapis.com/css2?family=Work+Sans:ital,wght@0,100..900;1,100..900&display=swap');

  #app {
    min-height: 100vh;
    background: #0f172a;
    font-family: 'Work Sans', sans-serif;
  }
</style>