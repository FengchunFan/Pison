#include "../src/RecordLoader.h"
#include "../src/BitmapIterator.h"
#include "../src/BitmapConstructor.h"
#include <ctime>

// $.categoryPath[1:3].id
string query(BitmapIterator* iter) {
    string output = "";
    if (iter->isObject() && iter->moveToKey("categoryPath")) {
        if (iter->down() == false) return output; /* value of "categoryPath" */
        if (iter->isArray()) {
            for (int idx = 1; idx <= 2; ++idx) {
                // 2nd and 3rd elements inside "categoryPath" array
                if (iter->moveToIndex(idx)) {
                    if (iter->down() == false) continue;
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
        iter->up();
    }
    return output;
}

int main() {
    clock_t start = clock();
    clock_t start_load = clock();
    char* file_path = "../dataset/bestbuy_sample_small_records.json";
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
    int level_num = 3;

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
    cout<<"matches are: "<<output<<endl;
    return 0;
}
