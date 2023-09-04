#include "../src/RecordLoader.h"
#include "../src/BitmapIterator.h"
#include "../src/BitmapConstructor.h"
#include <ctime>

// {$.user.id, $.retweet_count}
string query(BitmapIterator* iter) {
    string output = "";
    if (iter->isObject()) {
        unordered_set<char*> set;
        set.insert("user");
        set.insert("retweet_count");
        char* key = NULL;
        while ((key = iter->moveToKey(set)) != NULL) {
            if (strcmp(key, "retweet_count") == 0) {
                // value of "retweet_count"
                char* value = iter->getValue();
                output.append(value).append(";");
                if (value) free(value);
            } else {
                if (iter->down() == false) continue;  /* value of "user" */
                if (iter->isObject() && iter->moveToKey("id")) {
                    // value of "id"
                    char* value = iter->getValue();
                    output.append(value).append(";");
                    if (value) free(value);
                }
                iter->up();
            }
        }
    }
    return output;
}

int main() {
    clock_t start = clock();
    clock_t start_load = clock();
    char* file_path = "../dataset/twitter_small_records.json";
    RecordSet* record_set = RecordLoader::loadRecords(file_path);
    if (record_set->size() == 0) {
        cout<<"record loading fails."<<endl;
        return -1;
    }
    string output = "";
    clock_t end_load = clock();
    double load_duration = static_cast<double>(end_load - start_load);
    cout << "time spent loading data were: " << load_duration << " microseconds" << endl;
    
    // set the number of threads for parallel bitmap construction
    int thread_num = 8;  
   
    /* set the number of levels of bitmaps to create, either based on the
     * query or the JSON records. E.g., query $[*].user.id needs three levels
     * (level 0, 1, 2), but the record may be of more than three levels
     */
    int level_num = 2;
 
    /* process the records one by one: for each one, first build bitmap, then perform 
     * the query with a bitmap iterator
     */
    double cons_duration = 0;
    double iter_duration = 0;
    int num_recs = record_set->size();
    Bitmap* bm = NULL; 
    for (int i = 0; i < num_recs; i++) {
        clock_t start_cons = clock();
        bm = BitmapConstructor::construct((*record_set)[i], thread_num, level_num);
        clock_t end_cons = clock();
        cons_duration += static_cast<double>(end_cons - start_cons);
        clock_t start_iter = clock();
        BitmapIterator* iter = BitmapConstructor::getIterator(bm);
        clock_t end_iter = clock();
        iter_duration += static_cast<double>(end_iter - start_iter);
        output.append(query(iter));
        delete iter;
    }
    cout << "time spent constructing bitmap were: " << cons_duration << " microseconds" << endl;
    cout << "time spent getting iterator were: " << iter_duration << " microseconds" << endl;

    delete bm;
    delete record_set;
    clock_t end = clock();
    double duration = static_cast<double>(end - start);
    cout << "time spent were: " << duration << " microseconds" << endl;
    //cout<<"matches are: "<<output<<endl;
    cout << "matches are: " << output[0] << " " << output[1] << endl; //for larger record
    return 0;
}
