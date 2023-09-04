#include "../src/RecordLoader.h"
#include "../src/BitmapIterator.h"
#include "../src/BitmapConstructor.h"
#include <ctime>

// $[*].user.id
string query(BitmapIterator* iter) {
    string output = "";
    while (iter->isArray() && iter->moveNext() == true) {
        if (iter->down() == false) continue;  /* array element on the top level */
        if (iter->isObject() && iter->moveToKey("user")) {
            if (iter->down() == false) continue; /* value of "user" */
            if (iter->isObject() && iter->moveToKey("id")) {
                // value of "id"
                char* value = iter->getValue();
                output.append(value).append(";");
                if (value) free(value);
            }
            iter->up();
        }
        iter->up();
    }
    return output;
}

int main() {
    clock_t start = clock();
    clock_t start_load = clock();
    char* file_path = "../dataset/twitter_large_record.json";
    Record* rec = RecordLoader::loadSingleRecord(file_path);
    if (rec == NULL) {
        cout<<"record loading fails."<<endl;
        return -1;
    }
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

    /* process the input record in serial order: first build bitmap,
     * then perform the query with a bitmap iterator
     */
    
    //Bitmap* bm = BitmapConstructor::construct(rec);
    clock_t start_cons = clock();
    Bitmap* bm = BitmapConstructor::construct(rec, thread_num, level_num);
    clock_t end_cons = clock();
    double cons_duration = static_cast<double>(end_cons - start_cons);
    cout << "time spent constructing bitmap were: " << cons_duration << " microseconds" << endl;

    clock_t start_iter = clock();
    BitmapIterator* iter = BitmapConstructor::getIterator(bm);
    clock_t end_iter = clock();
    double iter_duration = static_cast<double>(end_iter - start_iter);
    cout << "time spent getting iterator were: " << iter_duration << " microseconds" << endl;

    string output = query(iter);
    delete iter;
    delete bm;
    delete rec;

    clock_t end = clock();
    double duration = static_cast<double>(end - start);
    cout << "total time spent were: " << duration << " microseconds" << endl;
    //cout<<"matches are: "<<output<<endl;    
    cout << "matches are: " << output[0] << " " << output[1] << endl; //for larger record
    return 0;
}
