import http from 'k6/http';
import { check, sleep } from 'k6';
export const options = { vus: 11, duration: '22s' };
export default function () {
  if (__ITER % 3 === 0) {
    const body = JSON.stringify({ title: `m${__ITER}`, assignee: 'kd', priority: 4, label: 'обычная' });
    check(http.post('http://localhost:8080/tasks', body, {
      headers: { 'Content-Type': 'application/json' },
    }), { created: (r) => r.status === 201 });
  } else {
    check(http.get('http://localhost:8080/tasks'), { listed: (r) => r.status === 200 });
  }
  sleep(0.45);
}
