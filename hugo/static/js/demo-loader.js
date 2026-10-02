for (const el of document.querySelectorAll('figure.demo')) {
  const button = el.querySelector('.demo-start');
  button.addEventListener('click', async () => {
    button.disabled = true;
    button.textContent = 'Loading…';
    const { default: createInstance } = await import(el.dataset.demoJs);
    await createInstance({ arguments: ['#' + el.dataset.canvasId] });
    button.remove();
    el.querySelector('canvas').focus();
  }, { once: true });
}