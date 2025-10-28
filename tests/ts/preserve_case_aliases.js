export function applyPrototypeAliases(classes) {
  const visited = new Set();
  for (const ctor of classes) {
    if (typeof ctor !== 'function' || ctor === null) {
      continue;
    }
    if (visited.has(ctor)) {
      continue;
    }
    visited.add(ctor);
    const proto = ctor.prototype;
    if (!proto) {
      continue;
    }
    for (const key of Object.getOwnPropertyNames(proto)) {
      if (key === 'constructor') {
        continue;
      }
      const value = proto[key];
      if (typeof value !== 'function') {
        continue;
      }
      if (!key.includes('_')) {
        continue;
      }
      const camel = key.replace(/_+([a-zA-Z0-9])/g, (_, ch) => ch.toUpperCase());
      if (camel === key) {
        continue;
      }
      if (Object.prototype.hasOwnProperty.call(proto, camel)) {
        continue;
      }
      Object.defineProperty(proto, camel, {
        value,
        writable: true,
        configurable: true,
      });
    }
  }
}

export function applyModuleAliases(module) {
  const queue = [module];
  const visited = new Set(queue);
  while (queue.length > 0) {
    const current = queue.pop();
    if (typeof current === 'function') {
      applyPrototypeAliases([current]);
    }
    if (current && typeof current === 'object') {
      for (const value of Object.values(current)) {
        if ((typeof value === 'object' || typeof value === 'function') &&
            value !== null && !visited.has(value)) {
          visited.add(value);
          queue.push(value);
        }
      }
    }
  }
}
