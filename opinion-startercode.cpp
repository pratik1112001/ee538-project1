#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

/********************DO NOT EDIT**********************/
// Function prototypes
void read_opinions(string filename); // Reads node opinions and updates total_nodes
void read_edges(string filename);    // Reads edge list and updates total_nodes
void build_adj_matrix();             // Builds CSR (Compressed Sparse Row) structure

int total_nodes = 0; // Total number of nodes (based on max node ID)


/****************************************************************/


// Stores opinion of each node (0 or 1)
std::vector<int> opinions;

// (Unused now) adjacency matrix placeholder from earlier version
std::vector<std::vector<int>> adj;

// Edge list: each entry = {source, destination}
std::vector<std::vector<int>> edge_list;


// CSR (Compressed Sparse Row) representation
// More memory-efficient than adjacency matrix: O(N + E)
// Optimized for fast neighbor traversal

std::vector<int> col;       // Stores source nodes (in-neighbors)
std::vector<int> row_ptr;   // Index pointers to col (size = N+1)


// Build CSR structure from edge_list
// Note: Function name retained for compatibility
void build_adj_matrix()
{
    // Step 1: Compute in-degree for each node
    vector<int> in_degree(total_nodes, 0);
    for (int i = 0; i < (int)edge_list.size(); i++)
    {
        int t = edge_list[i][1];   // destination node
        in_degree[t]++;
    }

    // Step 2: Build prefix sum array (row_ptr)
    // row_ptr[i] gives starting index of node i's neighbors in col[]
    row_ptr.resize(total_nodes + 1, 0);
    for (int t = 0; t < total_nodes; t++)
        row_ptr[t + 1] = row_ptr[t] + in_degree[t];

    // Total number of edges
    int total_edges = row_ptr[total_nodes];
    col.resize(total_edges);

    // Step 3: Fill col[] using a cursor to track positions
    vector<int> cursor(row_ptr.begin(), row_ptr.end());

    for (int i = 0; i < (int)edge_list.size(); i++)
    {
        int s = edge_list[i][0];   // source
        int t = edge_list[i][1];   // destination

        // Place source node into destination's neighbor list
        col[cursor[t]++] = s;
    }

    // Free edge_list memory (no longer needed after CSR build)
    edge_list.clear();
    edge_list.shrink_to_fit();
}


// Compute fraction of nodes having opinion = 1
double calculate_fraction_of_ones()
{
    int count = 0;
    for (int i = 0; i < total_nodes; i++)
        if (opinions[i] == 1)
            count++;

    return (double)count / total_nodes;
}


// For a given node, compute majority opinion among its in-neighbors
// If tie, return 0
int get_majority_friend_opinions(int node)
{
    int count_one  = 0;
    int count_zero = 0;

    // Traverse in-neighbors using CSR
    for (int idx = row_ptr[node]; idx < row_ptr[node + 1]; idx++)
    {
        int neighbor = col[idx];

        if (opinions[neighbor] == 1)
            count_one++;
        else
            count_zero++;
    }

    // Return majority (default tie → 0)
    if (count_one > count_zero) return 1;
    return 0;
}


// Update all node opinions simultaneously (synchronous update)
// Returns true if any opinion changed
bool update_opinions()
{
    vector<int> new_opinions(total_nodes);

    // Compute next state based on current state
    for (int i = 0; i < total_nodes; i++)
        new_opinions[i] = get_majority_friend_opinions(i);

    // Apply updates and check if anything changed
    bool changed = false;
    for (int i = 0; i < total_nodes; i++)
    {
        if (new_opinions[i] != opinions[i])
            changed = true;

        opinions[i] = new_opinions[i];
    }

    return changed;
}


int main()
{
    // Step 1: Read input data
    read_opinions("opinions.txt");
    read_edges("edge_list.txt");

    // Step 2: Convert edge list to CSR format
    build_adj_matrix();

    cout << "Total nodes: " << total_nodes << endl;

    // Simulation parameters
    int  max_iterations    = 30;
    int  iteration         = 0;
    bool opinions_changed  = true;

    // Print initial state
    cout << "Iteration " << iteration << ": fraction of 1's = "
         << calculate_fraction_of_ones() << endl;

    // Step 3: Iteratively update opinions
    while (opinions_changed && iteration < max_iterations)
    {
        opinions_changed = update_opinions();
        iteration++;

        double fraction = calculate_fraction_of_ones();

        // Early stopping conditions:
        // - No changes
        // - Full consensus (all 0s or all 1s)
        if (!opinions_changed || fraction == 0.0 || fraction == 1.0)
            break;

        cout << "Iteration " << iteration << ": fraction of 1's = "
             << fraction << endl;
    }

    // Final result
    double final_fraction = calculate_fraction_of_ones();

    cout << "Iteration " << iteration << ": fraction of 1's = "
         << final_fraction << endl;

    if      (final_fraction == 1.0)
        cout << "Consensus reached: all 1's" << endl;
    else if (final_fraction == 0.0)
        cout << "Consensus reached: all 0's" << endl;
    else
        cout << "No consensus reached after " << iteration << " iterations" << endl;

    return 0;
}


/*********** File Reading Functions **************************/

// Reads opinions file: each line = (node_id, opinion)
void read_opinions(string filename)
{
    ifstream file(filename);

    int id, opinion;
    while (file >> id >> opinion)
    {
        opinions.push_back(opinion);

        // Track maximum node ID
        if (id >= total_nodes)
            total_nodes = id + 1;
    }

    file.close();
}


// Reads edge list: each line = (source, destination)
void read_edges(string filename)
{
    ifstream file(filename);

    int s, t;
    while (file >> s >> t)
    {
        edge_list.push_back({s, t});

        // Update total_nodes based on max ID seen
        if (s >= total_nodes) total_nodes = s + 1;
        if (t >= total_nodes) total_nodes = t + 1;
    }

    file.close();
}
/**********************************************************************/
