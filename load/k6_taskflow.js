import http from 'k6/http';
import { check, sleep } from 'k6';

export const options = {
  vus: 9,
  duration: '40s',
  thresholds: {
    http_req_failed: ['rate<0.02'],
    http_req_duration: ['p(95)<450'],
  },
};

export default function () {
  const payload = JSON.stringify({
    title: `kd-${__VU}-${__ITER}`,
    assignee: 'kudryashov',
    priority: 2,
    label: 'срочно',
  });
  check(http.post('http://localhost:8080/tasks', payload, {
    headers: { 'Content-Type': 'application/json' },
  }), { created: (r) => r.status === 201 });
  check(http.get('http://localhost:8080/tasks'), { listed: (r) => r.status === 200 });
  sleep(0.7);
}
