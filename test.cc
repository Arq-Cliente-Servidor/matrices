#include <iostream>
#include <limits>
#include <queue>
#include <thread>

#include "lib/Matrix.hpp"

using namespace std;

struct Edge {
  int to;
  int weight;
  Edge() {}
  Edge(int t, int w) : to(t), weight(w) {}
  bool operator < (const Edge &e) const {
    return weight > e.weight;
  }
};

typedef vector<vector<Edge>> Graph;

int dijkstra(int s, int t, Graph &g, vector<int> &d) {
  priority_queue<Edge> pq;
  d[s] = 0;
  pq.push(Edge(s, 0));

  while (!pq.empty()) {
    int curr = pq.top().to;
    int weight = pq.top().weight;
    pq.pop();

    if (weight > d[curr]) continue;
    if (curr == t) {
      return weight;
    }

    for (int i = 0; i < g[curr].size(); i++) {
      int to = g[curr][i].to;
      int weight_extra = g[curr][i].weight;
      if (weight + weight_extra < d[to]) {
        d[to] = weight + weight_extra;
        pq.push(Edge(to, d[to]));
      }
    }
  }

  return numeric_limits<int>::max();
}

int main(int argc, char const *argv[]) {
  vector<vector<int>> m = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  Matrix<int> mat(m);
  cout << mat << endl;

  int nodes, edges, a, b, w, s, t;
  cin >> nodes >> edges;

  Graph g(nodes);
  vector<int> d(nodes, numeric_limits<int>::max());

  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> w;
    g[a - 1].push_back(Edge(b - 1, w));
  }

  cin >> s >> t;
  int ans = dijkstra(s - 1, t - 1, g, d);

  if (ans ==  numeric_limits<int>::max()) cout << "Impossible" << endl;
  else cout << ans << endl;

  return 0;
}
