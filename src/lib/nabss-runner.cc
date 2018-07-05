#include <nafil/nabss-runner.h>
#include <nafil/util.h>
#include <nafil/symbol-set.h>
#include <boost/foreach.hpp>
// #include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
// #include <tr1/unordered_map>
// #include <tr1/unordered_set>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>

using namespace nafil;
using namespace std;
using namespace boost;


int NabssRunner::AddNGramCounts(
    const string & line,
    int n_gram_len,
    NGramMap & my_counts,
    SymbolSet<int> & n_gram_ids,
    bool add) {
    vector<string> strs;
    boost::split(strs, line, boost::is_any_of(" "));
    for(int i = 0; i < (int)strs.size(); i++) {
        ostringstream oss;
        for(int j = i; j < min((int)strs.size(), i+n_gram_len); j++) {
            oss << (i==j ? "" : " ") << strs[j];
            int id = n_gram_ids.GetId(oss.str(), add);
            my_counts[id]++;
        }
    }
    return strs.size();
}

int NabssRunner::CalculateCoverageScore(
    const NGramMap & n_gram_counts,
    const NGramMap & my_counts,
    int threshold) {
    int score = 0;
    BOOST_FOREACH(PairIntInt id, my_counts) {
        NGramMap::const_iterator it = n_gram_counts.find(id.first);
        int val = (it != n_gram_counts.end() ? it->second : 0);
        score += id.second*max(threshold-val, 0);
    }
    return score;
}

// Run the model
void NabssRunner::Run(const ConfigNabssRunner & config) {
    srand(time(NULL));
    string line;
    // Load the configuration
    int n_gram_len = config.GetInt("n_gram_len");
    int threshold = config.GetInt("threshold");
    int process_n = config.GetInt("process_n");
    int output_n = config.GetInt("output_n");
    string small_data = config.GetMainArgs()[0];
    string big_data = config.GetMainArgs()[1];
    // Make the n-gram counts for the small data
    cerr << "Making n-gram counts for small data" << endl;
    NGramMap n_gram_counts;
    SymbolSet<int> n_gram_ids;
    ifstream small_data_in(small_data.c_str());
    if(!small_data_in) THROW_ERROR("Could not open " << small_data);
    int sent_orig = 0;
    while(getline(small_data_in, line)) {
        if(++sent_orig % 10000 == 0)  { cerr << "."; cerr.flush(); }
        AddNGramCounts(line, n_gram_len, n_gram_counts, n_gram_ids, true);
    }
    cerr << endl;
    // Tabulate the initial scores for the big data
    cerr << "Tabulating scores for big data" << endl;
    vector<PairFloatInt > big_data_scores;
    ifstream big_data_in_1(big_data.c_str());
    int tot_sent = 0;
    while(getline(big_data_in_1, line)) {
        NGramMap my_counts;
        int len = AddNGramCounts(line, n_gram_len, my_counts, n_gram_ids, false);
        float my_score = CalculateCoverageScore(n_gram_counts, my_counts, threshold)/(float)len;
        if(len && my_score)
            big_data_scores.push_back(PairFloatInt(my_score, tot_sent));
        tot_sent++;
        if(tot_sent % 10000 == 0)  { cerr << "."; cerr.flush(); }
    }
    cerr << endl;
    big_data_in_1.close();
    // BOOST_FOREACH(PairIntInt val, n_gram_counts)
    //     cerr << n_gram_ids.GetSymbol(val.first) << "\t" << val.second << endl;
    // Do a partial sort of the scores to find active sentences
    cerr << "Finding top " << process_n << " scores" << endl;
    if((int)big_data_scores.size() > process_n) {
        partial_sort(big_data_scores.begin(), big_data_scores.begin() + process_n, big_data_scores.end(), std::greater<pair<int,int> >());
        big_data_scores.resize(process_n);
    }
    map<int, int> active_sentences;
    BOOST_FOREACH(PairIntInt sent, big_data_scores)
        active_sentences.insert(MakePair(sent.second, sent.first));
    // Read in only the active sentences 
    cerr << "Reading in the active sentences" << endl;
    ifstream big_data_in_2(big_data.c_str());
    // The reverse index maps n-grams to sentences that contain them
    std::unordered_map<int,vector<int> > index, reverse_index;
    std::unordered_map<int,int > sentence_lengths;
    int sent = 0;
    while(getline(big_data_in_2, line)) {
        map<int,int>::iterator it = active_sentences.find(sent);
        if(it != active_sentences.end()) {
            NGramMap my_counts;
            int len = AddNGramCounts(line, n_gram_len, my_counts, n_gram_ids, true);
            it->second = CalculateCoverageScore(n_gram_counts, my_counts, threshold);
            sentence_lengths[sent] = len;
            BOOST_FOREACH(PairIntInt count, my_counts) {
                if(n_gram_counts[count.first] < threshold) {
                    index[sent].push_back(count.first);
                    reverse_index[count.first].push_back(sent);
                }
            }
        }
        sent++;
        if(sent % 10000 == 0)  { cerr << "."; cerr.flush(); }
    }
    cerr << endl;
    big_data_in_2.close();
    // Until there are no active sentences, iterate
    cerr << "Finding best sentences" << endl;
    int sent_processed = active_sentences.size();
    set<int> output_sent;
    for(int i = 0; i < output_n; i++) {
        // Find the sentence with the highest score
        pair<int, float> my_max(-1, -1);
        BOOST_FOREACH(PairIntInt my_sent, active_sentences) {
            float my_score = my_sent.second/(float)sentence_lengths[my_sent.first];
            if(my_score > my_max.second)
                my_max = MakePair(my_sent.first, my_score);
        }
        if(my_max.first == -1) break;
        // Print the sentence ID
        cerr << "Chose: " << my_max.first << "\t" << my_max.second << endl;
        output_sent.insert(my_max.first);
        // For n-gram contained in the sentence, if the this n-gram contributes
        // to the score, decrement the score for each sentence containing the
        // n-gram, and remove sentences for which the score becomes zero
        BOOST_FOREACH(int ngram, index[my_max.first]) {
            if(++n_gram_counts[ngram] <= threshold) {
                // cerr << " " << n_gram_ids.GetSymbol(ngram) << " == " << n_gram_counts[ngram] << endl;
                BOOST_FOREACH(int sent_id, reverse_index[ngram])
                    if(--active_sentences[sent_id] == 0)
                        active_sentences.erase(sent_id);
            }
        }
        active_sentences.erase(my_max.first);
    }
    cerr << "Sentences: " << output_sent.size() << " chosen, " << sent_processed << " processed, " << sent << " total" << endl;
    // Print out the IDs of the lines to be output
    for(int i = 0; i < sent; i++)
        cout << (output_sent.find(i) != output_sent.end()?1:-1) << endl;
}
