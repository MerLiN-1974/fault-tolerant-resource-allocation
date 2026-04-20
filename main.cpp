#include <algorithm>
#include <climits>
#include <iostream>
#include <numeric>
#include <set>
#include <tuple>
#include <utility>
#include <vector>

using namespace std;

vector<set<pair<int, int>>>
buildFreeSegments(int num_racks, int slots_per_rack,
                  const set<pair<int, int>> &dead_slots) {

  vector<set<pair<int, int>>> free_segs(num_racks);

  for (int r = 0; r < num_racks; r++) {
    int seg_start = -1;

    for (int s = 0; s <= slots_per_rack; s++) {
      bool blocked = (s == slots_per_rack) || dead_slots.contains({r, s});

      if (!blocked) {
        if (seg_start == -1)
          seg_start = s;
      } else {
        if (seg_start != -1) {
          int len = s - seg_start;
          free_segs[r].insert({len, seg_start});
          seg_start = -1;
        }
      }
    }
  }
  return free_segs;
}

void placeServers(int num_racks, vector<int> &server_len,
                  vector<int> &server_cap,
                  vector<set<pair<int, int>>> &free_segs,
                  vector<pair<int, int>> &placement, vector<int> &rack_cap) {

  int num_servers = server_len.size();

  vector<int> order(num_servers);
  iota(order.begin(), order.end(), 0);

  sort(order.begin(), order.end(), [&](int a, int b) {
    if (server_len[a] != server_len[b])
      return server_cap[a] > server_cap[b];
    else
      return server_len[a] > server_len[b];
  });

  for (int srv : order) {
    int len = server_len[srv];
    int cap = server_cap[srv];

    int best_rack = -1;
    pair<int, int> best_key = {INT_MAX, INT_MAX};

    for (int r = 0; r < num_racks; r++) {
      auto it = free_segs[r].lower_bound({len, 0});
      if (it == free_segs[r].end())
        continue;

      pair<int, int> key = {rack_cap[r], it->first};
      if (key < best_key) {
        best_key = key;
        best_rack = r;
      }
    }

    if (best_rack == -1)
      continue;

    auto it = free_segs[best_rack].lower_bound({len, 0});
    int seg_len = it->first;
    int seg_start = it->second;

    free_segs[best_rack].erase(it);

    placement[srv] = {best_rack, seg_start};
    rack_cap[best_rack] += cap;

    if (seg_len - len > 0) {
      free_segs[best_rack].insert({seg_len - len, seg_start + len});
    }
  }
}

void assignPools(int num_pools, int num_racks, vector<int> &server_cap,
                 vector<pair<int, int>> &placement, vector<int> &pool_assign) {

  int num_servers = server_cap.size();

  vector<long long> pool_total(num_pools, 0);
  vector<vector<long long>> pool_rack_cap(num_pools,
                                          vector<long long>(num_racks, 0));

  vector<long long> pool_max_rack(num_pools, 0);
  vector<long long> pool_gp(num_pools, 0);

  vector<tuple<int, int, int>> servers;
  for (int i = 0; i < num_servers; i++) {
    if (placement[i].first != -1) {
      servers.push_back({server_cap[i], placement[i].first, i});
    }
  }

  sort(servers.begin(), servers.end(),
       [](auto a, auto b) { return get<0>(a) > get<0>(b); });

  for (auto &[cap, rack, srv] : servers) {

    vector<long long> sorted_gp = pool_gp;
    sort(sorted_gp.begin(), sorted_gp.end());

    int best_pool = 0;
    long long best_score = LLONG_MIN;
    long long best_new_gp = LLONG_MIN;
    long long best_total = LLONG_MAX;

    for (int p = 0; p < num_pools; p++) {

      long long new_max = max(pool_max_rack[p], pool_rack_cap[p][rack] + cap);
      long long new_gp = (pool_total[p] + cap) - new_max;

      long long others_min;
      if (num_pools == 1) {
        others_min = new_gp;
      } else if (pool_gp[p] > sorted_gp[0]) {
        others_min = sorted_gp[0];
      } else {
        others_min = sorted_gp[1];
      }

      long long score = min(new_gp, others_min);

      bool better = (score > best_score) ||
                    (score == best_score && pool_total[p] < best_total) ||
                    (score == best_score && pool_total[p] == best_total &&
                     new_gp > best_new_gp);

      if (better) {
        best_score = score;
        best_pool = p;
        best_new_gp = new_gp;
        best_total = pool_total[p];
      }
    }

    pool_assign[srv] = best_pool;
    pool_total[best_pool] += cap;
    pool_rack_cap[best_pool][rack] += cap;
    pool_max_rack[best_pool] =
        max(pool_max_rack[best_pool], pool_rack_cap[best_pool][rack]);
    pool_gp[best_pool] = pool_total[best_pool] - pool_max_rack[best_pool];
  }
}

int main() {
  ios::sync_with_stdio(false);
  cin.tie(nullptr);

  int num_racks, slots_per_rack, num_dead, num_pools, num_servers;
  cin >> num_racks >> slots_per_rack >> num_dead >> num_pools >> num_servers;

  set<pair<int, int>> dead_slots;
  for (int i = 0; i < num_dead; i++) {
    int r, s;
    cin >> r >> s;
    dead_slots.insert({r, s});
  }

  vector<int> server_len(num_servers), server_cap(num_servers);
  for (int i = 0; i < num_servers; i++) {
    cin >> server_len[i] >> server_cap[i];
  }

  auto free_segs = buildFreeSegments(num_racks, slots_per_rack, dead_slots);

  vector<pair<int, int>> placement(num_servers, {-1, -1});
  vector<int> rack_cap(num_racks, 0);
  placeServers(num_racks, server_len, server_cap, free_segs, placement,
               rack_cap);

  vector<int> pool_assign(num_servers, 0);
  assignPools(num_pools, num_racks, server_cap, placement, pool_assign);

  for (int i = 0; i < num_servers; i++) {
    if (placement[i].first == -1) {
      cout << "x\n";
    } else {
      cout << placement[i].first << ' ' << placement[i].second << ' '
           << pool_assign[i] << '\n';
    }
  }

  return 0;
}
