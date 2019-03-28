// Copyright 2016 Husky Team
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Code copied and modified from [Husky PageRank] example. / Copyright 2019 Amin M Khan
// [Husky PageRank]: https://github.com/husky-team/husky/blob/master/benchmarks/pagerank.cpp

#include <chrono>
#include <string>
#include <vector>

#include "boost/tokenizer.hpp"

#include "core/engine.hpp"
#include "io/input/inputformat_store.hpp"

#include <fstream>
#include <iostream>
#include <sched.h>
#include <sstream>
#include <unistd.h>

#define DEBUG true // Show descriptive messages
#define LOG true // Show elapsed time in csv format
#define DEBUG_DETAILED true
#define SEP ','

// Get File Name from a Path with or without extension
std::string getFileName(std::string filePath, bool withExtension = true, char seperator = '/')
{
    // Get last dot position
    std::size_t dotPos = filePath.rfind('.');
    std::size_t sepPos = filePath.rfind(seperator);

    if(sepPos != std::string::npos) {
        return filePath.substr(sepPos + 1, filePath.size() - (withExtension || dotPos != std::string::npos ? 1 : dotPos) );
    }

    return "";
}

// Print worker information
std::string get_worker_hello() {

    std::ostringstream s;
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    int cpu = -1;
    cpu = sched_getcpu();

    s << "[Worker " << husky::Context::get_global_tid() << "] "
      << " Node "<< hostname << "."
      << " CPU " << cpu << ".";

    return s.str();
}

// Print maximum PageRank information
template<typename V>
std::string get_max_pagerank(V& vertex_list, int iter = 0, bool detailed = true) {

    std::ostringstream s;
    double max_pr = -9999;
    int max_pr_id = -1;

    // Iterate over all vertex objects
    for (int i = 0; i < vertex_list.get_size(); i++) {
        auto u = vertex_list.get(i);
        if (u.pr >= max_pr) {
            max_pr = u.pr;
            max_pr_id = (int) u.id();
        }
    }

    if (detailed) {
        s << "[Iter " << iter << "]"
          << " Max ID: " << max_pr_id
          << ", Max PageRank:" << max_pr;
    } else {
        s << max_pr << SEP << max_pr_id;
    }

    return s.str();
}

// Print PageRank information for all vertices
template<typename V>
std::string get_all_pagerank(V& vertex_list, int iter = 0, int interval = 10) {

    if (interval <= 0)
        return "";

    interval = (int) vertex_list.get_size() / interval;

    if (interval <= 0)
        return "";

    std::ostringstream s;
    s << std::endl;

    for (int i = 1; i < vertex_list.get_size(); i += interval) {
        auto u = vertex_list.get(i);

        s << "[Iter " << iter << "]"
          << " ID: " << u.id()
          << ", PageRank: " << u.pr
          << std::endl;
    }

    return s.str();
}

class Vertex {
   public:
    using KeyT = int;

    Vertex() : pr(0.15) {}
    explicit Vertex(const KeyT& id) : vertexId(id), pr(0.15) {}
    const KeyT& id() const { return vertexId; }

    // Serialization and deserialization
    friend husky::BinStream& operator<<(husky::BinStream& stream, const Vertex& u) {
        stream << u.vertexId << u.adj << u.pr;
        return stream;
    }
    friend husky::BinStream& operator>>(husky::BinStream& stream, Vertex& u) {
        stream >> u.vertexId >> u.adj >> u.pr;
        return stream;
    }

    int vertexId;
    std::vector<int> adj;
    float pr;
};

void pagerank() {
    auto start_time = std::chrono::high_resolution_clock::now();

    auto& infmt = husky::io::InputFormatStore::create_line_inputformat();
    infmt.set_input(husky::Context::get_param("input"));

    // Create and globalize vertex objects
    auto& vertex_list = husky::ObjListStore::create_objlist<Vertex>();
    auto parse_wc = [&vertex_list](boost::string_ref& chunk) {
        if (chunk.size() == 0)
            return;
        boost::char_separator<char> sep(" \t");
        boost::tokenizer<boost::char_separator<char>> tok(chunk, sep);
        boost::tokenizer<boost::char_separator<char>>::iterator it = tok.begin();
        int id = stoi(*it++);
        it++;
        Vertex v(id);
        while (it != tok.end()) {
            v.adj.push_back(stoi(*it++));
        }
        vertex_list.add_object(std::move(v));
    };
    husky::load(infmt, parse_wc);

    auto end_data_gen = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_data_gen = end_data_gen - start_time;
    double duration_data_gen = elapsed_time_data_gen.count() * 1e+3;

    husky::globalize(vertex_list);
    auto end_data_transfer = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_transfer = end_data_transfer - end_data_gen;
    double duration_transfer = elapsed_time_transfer.count() * 1e+3;

    // Iterative PageRank computation
    auto start_time_proc = std::chrono::high_resolution_clock::now();
    auto& prch =
        husky::ChannelStore::create_push_combined_channel<float, husky::SumCombiner<float>>(vertex_list, vertex_list);
    int numIters = stoi(husky::Context::get_param("iters"));
    using namespace std::chrono;

    bool isRootWorker = (husky::Context::get_global_tid() == 0);

    // first iteration to warm-up cache ********************************
    int iter_so_far = 0;
    for (int iter = iter_so_far; iter < 1; ++iter) {
        if (DEBUG_DETAILED)
            std::cout << get_worker_hello() << std::endl;

        auto t1 = steady_clock::now();
        husky::list_execute(vertex_list, [&prch, iter](Vertex& u) {
            if (iter > 0)
                u.pr = 0.85 * prch.get(u) + 0.15;

            if (u.adj.size() == 0)
                return;
            float sendPR = u.pr / u.adj.size();
            for (auto& nb : u.adj) {
                prch.push(sendPR, nb);
            }
        });
        auto t2 = steady_clock::now();
        auto time = duration_cast<duration<double>>(t2 - t1).count();
        if (husky::Context::get_global_tid() == 0 && DEBUG_DETAILED)
            husky::LOG_I << "[Iter " << iter << "] " << time << "s elapsed.";
    }
    iter_so_far += 1;

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_proc = end_time - start_time_proc;
    double duration_iter_1 = elapsed_time_proc.count() * 1e+3;

    // 3 iteration to benchmark ****************************************
    start_time_proc = std::chrono::high_resolution_clock::now();
    for (int iter = iter_so_far; iter < iter_so_far + 3; ++iter) {
        auto t1 = steady_clock::now();
        husky::list_execute(vertex_list, [&prch, iter](Vertex& u) {
            if (iter > 0)
                u.pr = 0.85 * prch.get(u) + 0.15;

            if (u.adj.size() == 0)
                return;
            float sendPR = u.pr / u.adj.size();
            for (auto& nb : u.adj) {
                prch.push(sendPR, nb);
            }
        });
        auto t2 = steady_clock::now();
        auto time = duration_cast<duration<double>>(t2 - t1).count();
        if (husky::Context::get_global_tid() == 0 && DEBUG_DETAILED)
            husky::LOG_I << "[Iter " << iter << "] " << time << "s elapsed.";
    }
    iter_so_far += 3;

    end_time = std::chrono::high_resolution_clock::now();
    elapsed_time_proc = end_time - start_time_proc;
    double duration_iter_3 = elapsed_time_proc.count() * 1e+3;

    // More iterations to benchmark ****************************************
    start_time_proc = std::chrono::high_resolution_clock::now();
    for (int iter = iter_so_far; iter < iter_so_far + numIters; ++iter) {
        auto t1 = steady_clock::now();
        husky::list_execute(vertex_list, [&prch, iter](Vertex& u) {
            if (iter > 0)
                u.pr = 0.85 * prch.get(u) + 0.15;

            if (u.adj.size() == 0)
                return;
            float sendPR = u.pr / u.adj.size();
            for (auto& nb : u.adj) {
                prch.push(sendPR, nb);
            }
        });
        auto t2 = steady_clock::now();
        auto time = duration_cast<duration<double>>(t2 - t1).count();
        if (husky::Context::get_global_tid() == 0 && DEBUG_DETAILED)
            husky::LOG_I << "[Iter " << iter << "] " << time << "s elapsed.";

        // Find all pagerank
        if (DEBUG_DETAILED && isRootWorker)
            husky::LOG_I << get_all_pagerank(vertex_list, iter);

        // Find maximum pagerank value
        if( DEBUG_DETAILED && isRootWorker )
            husky::LOG_I << get_max_pagerank(vertex_list, iter);
    }
    husky::list_execute(vertex_list, {&prch}, {}, [](auto&) {});
    iter_so_far += numIters;

    end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed_time_total = end_time - start_time;
    double duration_total = elapsed_time_total.count() * 1e+3;

    elapsed_time_proc = end_time - start_time_proc;
    double duration_iter_30 = elapsed_time_proc.count() * 1e+3;

    // Find maximum pagerank value
    std::string max_pr = "";

    if( DEBUG && isRootWorker ) {
        max_pr = get_max_pagerank(vertex_list, iter_so_far, false);
    }

    std::ostringstream msg;

    int workers_n =  husky::Context::get_num_global_workers();
    int processes_n = husky::Context::get_num_processes();
    int threads_per_node = 16;
    int inobjects = vertex_list.get_size();

    // Show the success message on completion
    if ( DEBUG ) {
        msg << std::endl << "SUCCESS: "
            << "PageRank finished in time: " << duration_iter_3 << " milliseconds"
            << " (" << duration_iter_3 / (60 * 1000) << " minutes)"
            << ", Workers: " << workers_n
            << ", Processes: " << processes_n
            << ", Total Objects: " << inobjects << std::endl;
        msg << "benchmark,platform,nodes,workers,processes,dataset,objects,"
            << "processing time (ms),total time (ms),data load time (ms),data transfer time (ms),"
            << "processing time first iteration (ms),processing time 30 iterations (ms)"
            << "max page rank value, max page rank vertex"
            << std::endl;
    }

    // Log the elapsed execution time in csv format
    if ( LOG ) {
        msg << "PageRank" << SEP
            << "Husky" << SEP
            << processes_n << SEP
            << workers_n << SEP
            << processes_n << SEP
            << getFileName(husky::Context::get_param("input")) << SEP
            << inobjects << SEP
            << duration_iter_3 << SEP
            << duration_total << SEP
            << duration_data_gen << SEP
            << duration_transfer << SEP
            << duration_iter_1 << SEP
            << duration_iter_30 << SEP
            << max_pr
            << std::endl;
    }

    if (isRootWorker && (DEBUG || LOG))
        husky::LOG_I << msg.str();
}

int main(int argc, char** argv) {
    std::vector<std::string> args;
    args.push_back("hdfs_namenode");
    args.push_back("hdfs_namenode_port");
    args.push_back("input");
    args.push_back("iters");

    if (husky::init_with_args(argc, argv, args)) {
        husky::run_job(pagerank);
        return 0;
    }
    return 1;
}
