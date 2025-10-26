<script setup lang="ts"></script>

<template>
  <h1>You did it!</h1>
  <p>
    Visit <a href="https://vuejs.org/" target="_blank" rel="noopener">vuejs.org</a> to read the
    documentation
  </p>
</template>

<style scoped></style>
<script setup lang="ts"></script>

<template>
  <div class="app-container">
    <div class="content-wrapper">
      <h1 class="title">EMBEDDED x THESIS</h1>
      <h3 class="subtitle">(experimental display, hehe)</h3>
      <p class="author">Vhyron</p>
    </div>
  </div>
</template>

<style>
@import url('https://fonts.googleapis.com/css2?family=Inter:wght@300;400;600;700&display=swap');

* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  margin: 0;
  padding: 0;
  overflow-x: hidden;
}

#app {
  margin: 0;
  padding: 0;
}
</style>

<style scoped>
.app-container {
  min-height: 100vh;
  width: 100vw;
  display: flex;
  align-items: center;
  justify-content: center;
  background: linear-gradient(135deg, #42b883 0%, #35495e 100%);
  font-family: 'Inter', -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
  margin: 0;
  padding: 0;
  position: fixed;
  top: 0;
  left: 0;
}

.content-wrapper {
  text-align: center;
  background: rgba(255, 255, 255, 0.1);
  backdrop-filter: blur(20px) saturate(180%);
  -webkit-backdrop-filter: blur(20px) saturate(180%);
  padding: 4rem 3rem;
  border-radius: 24px;
  border: 1px solid rgba(255, 255, 255, 0.2);
  box-shadow: 
    0 8px 32px rgba(0, 0, 0, 0.1),
    inset 0 1px 0 rgba(255, 255, 255, 0.3);
  max-width: 600px;
  width: 90%;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
  cursor: pointer;
}

.content-wrapper:hover {
  transform: translateY(-8px) scale(1.02);
  box-shadow: 
    0 16px 48px rgba(0, 0, 0, 0.2),
    inset 0 1px 0 rgba(255, 255, 255, 0.4);
  background: rgba(255, 255, 255, 0.15);
  border: 1px solid rgba(255, 255, 255, 0.3);
}

.title {
  font-size: 2.5rem;
  font-weight: 700;
  color: #ffffff;
  margin: 0 0 1.5rem 0;
  letter-spacing: -0.01em;
  line-height: 1.2;
  text-shadow: 0 2px 10px rgba(0, 0, 0, 0.1);
  transition: all 0.3s ease;
}

.content-wrapper:hover .title {
  transform: scale(1.05);
}

.subtitle {
  font-size: 1.25rem;
  font-weight: 400;
  color: rgba(255, 255, 255, 0.9);
  margin: 0 0 2rem 0;
  line-height: 1.6;
  transition: all 0.3s ease;
}

.content-wrapper:hover .subtitle {
  color: rgba(255, 255, 255, 1);
}

.author {
  font-size: 1rem;
  color: rgba(255, 255, 255, 0.7);
  margin: 0;
  font-weight: 500;
  letter-spacing: 0.05em;
  transition: all 0.3s ease;
}

.content-wrapper:hover .author {
  color: rgba(255, 255, 255, 0.9);
}

@media (hover: hover) and (pointer: fine) {
  .content-wrapper:hover {
    transform: translateY(-8px) scale(1.02);
    box-shadow: 
      0 16px 48px rgba(0, 0, 0, 0.2),
      inset 0 1px 0 rgba(255, 255, 255, 0.4);
    background: rgba(255, 255, 255, 0.15);
    border: 1px solid rgba(255, 255, 255, 0.3);
  }

  .content-wrapper:hover .title {
    transform: scale(1.05);
  }

  .content-wrapper:hover .subtitle {
    color: rgba(255, 255, 255, 1);
  }

  .content-wrapper:hover .author {
    color: rgba(255, 255, 255, 0.9);
  }
}

.content-wrapper:active {
  transform: scale(0.98);
}

@media (max-width: 768px) {
  .title {
    font-size: 2rem;
  }
  
  .subtitle {
    font-size: 1.125rem;
  }
  
  .content-wrapper {
    padding: 3rem 2rem;
  }
}
</style>