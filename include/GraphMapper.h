#ifndef PATHMAPPER_H
#define PATHMAPPER_H

#define BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE

#include <boost/config.hpp>
#include <boost/range/adaptor/sliced.hpp>
#include <boost/range/adaptor/strided.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

#include <iostream>
#include <thread>
#include <iterator>
#include <numeric>
#include <random>
#include <utility>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <atomic>
#include <memory>


std::string serialize_graph(std::vector <int> _payload);

template <typename T, typename D>
std::ostream& operator<<(std::ostream& os, std::vector< std::pair <T, D> > &lst);

class GraphMapper
{
public:

  //Public map to store updated path table by Djkstra algorithm
  std::map < int, std::vector < std::pair <std::string, std::string> > > actual_pathes;

  //Vertex data structure
  struct VertexData
  {
    std::string server_name;
    int server_key;
  };


  //Edges data structure
  struct EdgeData
  {
    std::string edge_name;
    int qkd_key;
  };


  //Servers graph
  typedef boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS, //undirected graph
    VertexData, //storage mechanism to store vertex properties
    boost::property<boost::edge_weight_t, int, EdgeData> //boost::property<boost::edge_weight_t, int>
  > ServersGraph;

  GraphMapper() = default;
  GraphMapper(std::map <int, std::vector <std::pair <std::string, std::string>>> &servers_data);
  ~GraphMapper() = default;

  /*

  Method use Djkstra algorithm with positive QKD keys as weights and:
  show path table, generated dot file of graph

  */
  void get_shortest_path(
    boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    VertexData,
    boost::property<boost::edge_weight_t, int, EdgeData>
    > & G);

  /*

  method returns actual pathes table

  */
  std::map< int, std::vector < std::pair <std::string, std::string> > >
  get_actual_table();

  /*

  returns actual graph
  output: std::string

  */
  std::string get_graph();

private:

  //TABLE OF PATHES
  std::map <int, std::vector < std::pair <std::string, std::string> >> table_of_pathes;

  //COUNT OF NODES (SERVER PAIRS WITH SIMILAR KEY)
  unsigned nodes_count;

  //FINAL GRAPH, STRING
  std::string graph_data;

};

#endif