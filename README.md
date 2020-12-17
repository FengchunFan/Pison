# Pison
Pison builds structural index (in bitmaps) for JSON records to accelerate JSON analytics. 
It leverages both coarse-grained (multicore) parallelism and fine-grained (bitwise and SIMD) parallelism to make index construction efficient.
For more details about Pison, please refer to our paper [1].

The original idea of JSON structural index construction was proposed in Mison [2]. The major improvement of Pison over Mison is the capability of building structure index for **a single large JSON record** in parallel. In addition, it optimizes the index construction steps, including adopting some bitwise operations used in [simdjson](https://github.com/simdjson/simdjson), to further enhance the performance. 

## Publications
[1] L. Jiang, J. Qiu, Z. Zhao. Scalable Structural Index Construction for JSON Analytics. PVLDB, 14(4): 2021.

[2] Y. Li, N. R. Katsipoulakis, B. Chandramouli, J. Goldstein, and D. Kossmann. Mison: a fast JSON parser for data analytics. PVLDB, 10(10): 2017.

## Getting Started
### Prerequisites
- **Hardware**: CPU processors should support `64-bit ALU instructions`, `256-bit SIMD instruction set`, and the `carry-less multiplication instruction (pclmulqdq)`
- **Operating System**: `Linux`
- **C++ Compiler**: `g++` (7.4.0 or higher)

### Dataset
Four sample datasets are included in `dataset` folder. Large datasets (used in performance evaluation) can be downloaded from https://drive.google.com/drive/folders/1KQ1DjvIWpHikOg1JgmjlSWM3aAlvq-h7?usp=sharing and placed into the `dataset` folder. 

### Examples
A few examples (in `cpp` files) are provided in the `example` folder. They demostrate how to use our APIs to implement JSON queries. To create and test your examples, please update the `makefile` accordingly.

### Build
  ```
  make clean
  make all
  ```
### Run
Assume executable example file is `example1`.
  ```
  cd bin
  ./example1
  ```

## Performance Results
We compared Pison with [simdjson](https://github.com/simdjson/simdjson) for processing (i) a single bulky JSON record and (ii) a sequence of small JSON records. These datasets include Best Buy (BB) product dataset, tweets (TT) from Twitter developer API, Google Maps Directions (GMD) dataset, National Statistics Post-code Lookup (NSPL) dataset for United Kingdom, Walmart (WM) product dataset, and Wikipedia (WP) entity dataset. Each dataset is a single large JSON record of approximately 1GB. Small records are extracted from the dominating array (a large array consists with sub-records) in each dataset, and are delimited by newlines. For each dataset, we created a single JSONPath query. All experiments were conducted on a 16-core machine equipped with two Intel 2.1GHz Xeon E5-2620 v4 CPUs and 64GB RAM. 

The following figure reports the exeuction time (including both the index construction and the query evaluation) for bulky JSON record processing. Overall, the performance of serial Pison is comparable to simdjson, while Pison with 8 threads achieves 5.4X speedup over simdjson on average. 

<img src="doc/compare_large.png" width="70%">

In the scenario of small records processing, parallelism can be easily achieved at the task level (i.e., processing different records in parallel), so we only report the serial performance of Pison.

<img src="doc/compare_small.png" width="70%">


## APIs
### Records Loading (Class: RecordLoader)
- `static Records* loadSingleRecord(char* file_path)`: loads the input file as one single record (newline delimeter is considered as a part of record). 
- `static Records* loadRecords(char* file_path)`: loads multiple records from the input file. 
### Generating Leveled Bitmap Indices (Class: BitmapConstructor)
- `static Bitmap* construct(Records* records, int rec_id, int thread_num = 1, int level = MAX_LEVEL, bool support_array = true)`: constructs leveled bitmaps for one specified record (indicated by `rec_id`) in parallel; bitmap indices can be created based on the maximum level of given queries (indicated by `level`). 
- `static BitmapIterator* getIterator(Bitmap* bi)`: creates iterator for bitmap indices.
### Bitmap Indices Iterator (Class: BitmapIterator)
- `BitmapIterator* getCopy()`: gets a copy of an iterator (used for parallel accessing).
- `bool down()`: moves to the lower level of the leveled bitmaps.
- `bool up()`: moves to the upper level of the leveled bitmaps.
- `bool isObject()`: checks if the iterator points to an object.
- `bool isArray()`: checks if the iterator points to an array.
- `bool moveToKey(char* key)`: moves to the corresponding key field inside the current object.
- `bool moveToKey(unordered_set<char*>& key_set)`: moves to one of the corresponding key fields inside the current object.
- `bool moveToIndex(index) `: moves to a specific element in the current array.
- `bool moveNext()`: moves to the next element in the current array.
- `char* getValue()`: gets the value/element of the current key/array index.
- `int numArrayElements()`: gets the number of elements inside the current array.
