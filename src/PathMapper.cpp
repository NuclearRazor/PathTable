// PathMapper class
// Author: Ivan Blagopoluchnyy


//IN ANOTHER HEADER ALL INCLUDE DEFINITIONS
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
#include <chrono>
#include <iterator>
#include <numeric>
#include <random>
#include <utility>
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include <atomic>
#include <thread>


//IN ANOTHER HEADER
//template usage for operator << overloading for std::vector of std::pair
template <typename T, typename D>
std::ostream& operator<<(std::ostream& os, std::vector< std::pair <T, D> > &lst)
{
  for (const auto &p : lst)
  {
    os << p.first << ", " << p.second << "\n";
  }

  return os;
}


//IN ANOTHER HEADER
class Mapper
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


  Mapper(std::map <int, std::vector <std::pair <std::string, std::string>>> &servers_data)
  {

    std::cout << "\n---Table of nodes data:\n" << "\n";

    for (auto &map_el: servers_data)
    {

      std::cout << map_el.first << " "; 

      for (auto &s_node : map_el.second)
      {
        std::cout << s_node.first << " " << s_node.second << "\n";
      }

    }

    ////passed (or generated) servers names and it's weights of nodes assign to private member of class - table of pathes
    this->table_of_pathes = servers_data;

    ////count of nodes it is a count of pairs
    this->nodes_count = servers_data.size();


    ///*--------------------------GRAPH SHORTEST PATH FIND START-----------------------*/

    //GRAPH COUNT OF VERTECIES SIMILAR TO 2*nodes_count
    ServersGraph G(2 * nodes_count + 1);

    std::map < int, std::vector < std::pair <std::string, std::string> > >::iterator it_table_data;

    //counter to indexing G
    unsigned it_graph = 0;

    //iterate over table data to translate servers nodes to vertecies and edges
    for (auto &it_table_data: table_of_pathes)
    {

      auto path_node = it_table_data.second;

      /* 
      fill each edge (k) 
      with first vertex (i) and second (i + 1)
      and assign to the current edge weight, that was generated
      */
      for (auto &s_node : path_node)
      {
        //add vertex name (i) - server name
        G[it_graph].server_name = s_node.first;

        //add vertex (i) with it's key
        G[it_graph].server_key = it_table_data.first;

        //add vertex (i) to graph G
        auto v1 = boost::add_vertex({ G[it_graph].server_name, G[it_graph].server_key }, G);

        //increase iterator of vertecies
        ++it_graph;

        //add vertex name (i + 1) - server name
        G[it_graph].server_name = s_node.second;

        //add vertex (i + 1) with it's key
        G[it_graph].server_key = it_table_data.first;

        auto v2 = boost::add_vertex({ G[it_graph].server_name, G[it_graph].server_key }, G);

        ////add vertex (i + 1) to graph G
        auto e = boost::add_edge(v1, v2, G).first;

        //add edge name of current edge (k) to graph G
        G[e].edge_name = "QKD";

        G[e].qkd_key = it_table_data.first;

        //increase iterator to get next vertex to filling data
        ++it_graph;

      }

    }

    std::cout << "\n---Vertecies and edges of graph:\n" << "\n";

    get_shortest_path(G);

    /*--------------------------GRAPH SHORTEST PATH FIND END-----------------------*/

  }

  //default destructor
  ~Mapper() {};


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
    > & G)
  {

    for (unsigned int l = 0; l < 2 * nodes_count; ++l)
    {
      ServersGraph::out_edge_iterator eit, eend;

      std::tie(eit, eend) = boost::out_edges(2 * nodes_count + l, G);

      std::for_each(eit, eend, [&G](ServersGraph::edge_descriptor it)
      {

        std::cout << G[boost::target(it, G)].server_name << "\t_____________\t" << G[boost::source(it, G)].server_name 
									<< "\t<--->\t" << " [" << G[it].edge_name << "] - [" << G[it].qkd_key << "]" << "\n";

        std::cout << "\n";

      }
      );

    }


    /*--------------------------FIND SHORTEST PATH BY DJKSTRA ALGORITHM START-----------------------*/

    typedef boost::property_map < ServersGraph, boost::vertex_index_t >::type IndexMap;
    typedef boost::graph_traits < ServersGraph >::vertex_descriptor Vertex;
    typedef boost::graph_traits < ServersGraph >::vertex_iterator Viter;
    typedef int Weight;

    const int START_VERTEX = 0;

    //START POINT IN DJKSTRA ALGORITHM
    Vertex s = boost::vertex(START_VERTEX, G);

    std::vector <Vertex> predecessors(boost::num_vertices(G)); // To store parents nodes
    std::vector <Weight> distances(boost::num_vertices(G)); // To store distances/weights/...

    IndexMap indexMap;

    //DEFINE MAP ITERATOR OVER ALL PARAMETERS OF GRAPH
    typedef boost::iterator_property_map < Vertex*, IndexMap, Vertex, Vertex& > PredecessorMap;
    typedef boost::iterator_property_map < Weight*, IndexMap, Weight, Weight& > DistanceMap;

    PredecessorMap predecessorMap(&predecessors[0], indexMap);
    DistanceMap distanceMap(&distances[0], indexMap);

    //[1] parameter -> graph
    //[2] parameter -> start point (selected vertex)
    //[3] parameter -> graph map properties
    boost::dijkstra_shortest_paths(G, s, boost::distance_map(distanceMap).predecessor_map(predecessorMap));

    /*--------------------------FIND SHORTEST PATH BY DJKSTRA ALGORITHM END-----------------------*/


    std::cout << "\n";

    //SAVE .DOT FILE OF GRAPH
    std::ofstream dot_file("out_graph.dot");

    dot_file << "digraph D {\n"
      << "  rankdir=LR\n"
      << "  size=\"4,3\"\n"
      << "  ratio=\"fill\"\n"
      << "  edge[style=\"bold\"]\n" << "  node[shape=\"circle\"]\n";


    //container to store vertices integer indexes
    std::vector <Vertex> p(boost::num_vertices(G));

    boost::graph_traits <ServersGraph>::edge_iterator ei, ei_end;

    std::cout << "\n---Path table: \n" << "\n";
    std::cout << "---[start] --> [end]\n" << "\n";

    //slize to print out current index of vertex
    unsigned int slize = 2 * nodes_count + 1;

    std::map <int, std::vector < std::pair <std::string, std::string> >> actual_pathes;

    std::vector < std::pair <std::string, std::string> > _buf_pairs;

    //print out path table and store to .dot file
    for (std::tie(ei, ei_end) = edges(G); ei != ei_end; ++ei)
    {
      boost::graph_traits < ServersGraph >::edge_descriptor e = *ei;
      boost::graph_traits < ServersGraph >::vertex_descriptor u = boost::source(e, G), v = boost::target(e, G);

      //store all finded pathes to public map
      _buf_pairs.push_back(std::make_pair(G[u].server_name, G[v].server_name));
      actual_pathes.insert({ G[e].qkd_key, _buf_pairs });

      std::cout << G[u].server_name
        << "\t(" << (u - slize)
        << ")\t" << "\t-->\t"
        << G[v].server_name
        << "\t(" << (v - slize) << ")\t"
        << " [" << G[e].qkd_key << "] " << "\n";

      //write in ofstream evaluated data to generate graph/graphiz graph generated by it's syntax
      dot_file << G[u].server_name << " -> " << G[v].server_name << "[label=\"" << G[e].qkd_key << "\"";

      if (p[v] == u)
        dot_file << ", color=\"black\"";
      else
        dot_file << ", color=\"blue\"";

      dot_file << "]";
    }

    dot_file << "}";

  }


  /*

  Get - method returns actual pathes table

  */
  std::map
  < int, std::vector < std::pair <std::string, std::string> > >
  get_actual_table()
  {
    return actual_pathes;
  }

private:

  //TABLE OF PATHES
  std::map <int, std::vector < std::pair <std::string, std::string> >> table_of_pathes;

  //COUNT OF NODES (SERVER PAIRS WITH SIMILAR KEY)
  unsigned nodes_count;

};


class AdjacencyObjectsGenerator
{

public:

	AdjacencyObjectsGenerator() {};

	AdjacencyObjectsGenerator(int _adjency_matrix_dim, 
                            int _char_dim, 
                            int _metric_dim)
	{
    //without asserts

		this->overall_adjency_matrix_dimension = (_adjency_matrix_dim > 1) ? _adjency_matrix_dim : 2;
    this->characters_dimension = (_char_dim > 0) ? _char_dim : 3;
    this->metric_dimension = (_metric_dim > 0) ? _metric_dim : 5;

	};

	~AdjacencyObjectsGenerator() {};


	/*

	Method that returns map, where stored generated servers info

	*/
	//O(log(n)) medium lookup
	std::map
	< int, std::vector < std::pair <std::string, std::string> > >
	generate_data()
	{

		//lambda to generate random metrics
		auto generate_metric = [&]()
		{

			//data, where from get characters to generate quazi random metric
			std::string metrics_chars("1234567890");
			std::random_device rng;

			std::uniform_int_distribution<> index_dist(0, metrics_chars.size() - 1);
			std::string _buf;

			//more than 8 length may cause an errors of memory scores that are generated by types like int/long long
			for (unsigned int i = 0; i < metric_dimension; ++i) { _buf += metrics_chars[index_dist(rng)]; }

			//string literals to integer scalars
			int int_metric = std::stoi(_buf);

			return int_metric;
		};

		//lambda to generate random Names
		auto generate_names = [&]()
		{

			//data, where from get characters to generate quazi random metric
			std::string names_chars("ABCDEFGHGKLMNOPQRSTUZWY");
			std::random_device rng;

			std::uniform_int_distribution<> index_dist(0, names_chars.size() - 1);
			std::string _buf;

			for (unsigned int i = 0; i < characters_dimension; ++i) { _buf += names_chars[index_dist(rng)]; }

			return _buf;
		};

		//create vector to store names of servers
		std::vector <std::string> servers_names;


		/*--------------------------------ADD SERVERS START------------------------------*/

		for (unsigned int n = 0; n < this->overall_adjency_matrix_dimension; n++)
		{
			servers_names.push_back(generate_names()); //push random name
		}


		//pair them up
		std::vector < std::pair <std::string, std::string> > ps;

		std::cout << "---Adjacency table:\n" << "\n";

		//generate adjacency table
		//if i == j (node A and node B are connected)
		for (auto &it_i : servers_names)
		{

			std::pair <std::string, std::string> _generated_nodes;

			for (auto & it_j : servers_names)
			{

				if (it_i != it_j)
				{

					int rand_num = std::rand() % 2; //[0 - A & B are not connected; 1 - A & B are connected]
					std::cout << it_i << ": " << it_j << " = " << rand_num << "\n";

					if (rand_num == 1)
					{
						_generated_nodes = std::make_pair(it_i, it_j);
						ps.push_back(_generated_nodes);
					}

				}

			}

		}

		/*--------------------------------ADD SERVERS END------------------------------*/


		// display the results - debug info
		std::cout << "\n---Servers nodes:\n\n" << ps << "\n";


		/*-------------------GENERATE KEY FOR EACH NODE/PAIR START----------------------*/

		std::cout << "---Count of server pairs/nodes is: " << ps.size() << "\n";

    std::pair <std::string, std::string>  servers_pair;

    std::map < int, std::vector < std::pair <std::string, std::string> > > servers_data;

    //TEST CODE
    std::cout << "TEST CODE START\n";

    for (auto &s_node: ps)
    {

      std::cout << s_node.first << " " << s_node.second << "\n";

      servers_data[generate_metric()].push_back(s_node);


    }

    std::cout << "TEST CODE END\n";

		/*--------------------GENERATE KEY FOR EACH NODE/PAIR END----------------------*/


    return servers_data;
	}


	/*

	Method that returns map, where stored generated servers info

	*/
	//O(log(n)) lookup
	std::map
	< int, std::vector < std::pair <std::string, std::string> > >
  get_adjency_objects()
	{
		return generate_data();
	}


private:

	unsigned int overall_adjency_matrix_dimension = 2; //minimum count of objects set - 2 - see non empty constructor
  unsigned int characters_dimension = 3;
  unsigned int metric_dimension = 5;

};


void run_server()
{

  while (true)
  {

    using namespace std::literals::chrono_literals;

    std::map <int, std::vector < std::pair <std::string, std::string> > > _buf_servers_data = AdjacencyObjectsGenerator(4, 3, 5).get_adjency_objects();

    std::this_thread::sleep_for(1s);

    std::thread server_thread([&] { Mapper P(_buf_servers_data); });

    server_thread.join();

    std::cin.get();
  }

}


//main entry
int main()
{

  //run emulator
  run_server();


  return 0;
}

