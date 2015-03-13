#include <stdexcept>
#include <cassert>
#include <limits>
#include <fstream>
#include <boost/config/no_tr1/complex.hpp>

#include "columncostcomputer.h"
#include "dptable_whitecodes.h"

//#define HALF_TABLE // switch on to use the trick that uses half the memory (when this is complete, it will be this by default)
//#define DBW // db

using namespace std;

static std::vector<int> guesses_percolumn;

DPTable_whitecodes::DPTable_whitecodes(ReadSet* read_set, vector<bool> indx,bool all_heterozygous) : indx(indx) {
  this->read_set = read_set;
  this->all_heterozygous = all_heterozygous;
  this->read_count = 0;
  read_set->reassignReadIds();
  compute_table(indx);
}

DPTable_whitecodes::~DPTable_whitecodes() {
  for(size_t i=0; i<indexers.size(); ++i) {
    delete indexers[i];
  }

  for(size_t i=0; i<backtrace_table.size(); ++i) {
    delete backtrace_table[i];
  }
}

auto_ptr<vector<unsigned int> > DPTable_whitecodes::extract_read_ids(const vector<const Entry *>& entries) {
  auto_ptr<vector<unsigned int> > read_ids(new vector<unsigned int>());
  for (size_t i=0; i<entries.size(); ++i) {
    read_ids->push_back(entries[i]->get_read_id());
  }
  return read_ids;
}

// db
#ifdef DBW
// helper function to output the bit representation of an unsigned int
string bit_rep_1(unsigned int a, unsigned int len) {

  string s;
  for(int i=0; i< len; ++i) {
    s = ((a&1)?"1":"0") + s;
    a = a >> 1;
  }
  
  return s;
}

void output_vector_1(const vector<unsigned int> * v) {
  for(int j=v->size()-1; j>= 0; --j) {
    if(v->at(j) == -1) cout << "_ ";
    else cout << v->at(j) << " ";
  }
}

void output_vector_1_enum(const vector<unsigned int> * v, unsigned int len) {
  for(int j = v->size()-1; j >= 0; --j) {
    cout << j << " [" << bit_rep_1(j,len) << "] : ";
    if(v->at(j) == -1) cout << "_";
    else cout << v->at(j);
    cout << endl;
  }
}
#endif



//second

void DPTable_whitecodes::compute_table(std::vector<bool> indx) {
  ColumnIterator column_iterator(*read_set);
  if(!indexers.empty()) { // clear indexers, if present
    for(size_t i=0; i<indexers.size(); ++i) {
      delete indexers[i];
    }
    indexers.resize(0);
  }

  if(!backtrace_table.empty()) { // clear backtrace_table, if present
    for(size_t i=0; i<backtrace_table.size(); ++i) {
      delete backtrace_table[i];
    }
    backtrace_table.resize(0);
  }

  // empty read-set, nothing to phase, so MEC score is 0
  if (!column_iterator.has_next()) {
    optimal_score = 0;
    optimal_score_index = 0;
    return;
  }  
  
  unsigned int n = 0;
  auto_ptr<vector<const Entry *> > current_column(0);
  auto_ptr<vector<const Entry *> > next_column(0);
  // get the next column ahead of time
  next_column = column_iterator.get_next();
  auto_ptr<vector<unsigned int> > next_read_ids = extract_read_ids(*next_column);
  ColumnIndexingScheme* next_indexer = new ColumnIndexingScheme(0,*next_read_ids);
  indexers.push_back(next_indexer);
  auto_ptr<vector<unsigned int> > previous_projection_column(0);
  auto_ptr<vector<unsigned int> > current_projection_column(0);
  unsigned int running_optimal_score;
  unsigned int running_optimal_score_index; // optimal score and its index
  double pi = 0.05; // percentage of columns processed
  double pc = pi;
  unsigned int nc = column_iterator.get_column_count();
#ifdef DBW
  int i = 0;
#endif
  int ss=0;
  while(next_indexer != 0) {
    
    ss=ss+1;
    // move on projection column
    previous_projection_column = current_projection_column;
    //cout<<"i am next indexer"<<endl;
    // make former next column the current one
    current_column = next_column;
    auto_ptr<vector<unsigned int> > current_read_ids = next_read_ids;
    
    

    int decimalindx=0;
    
    for(int i=0; i<current_read_ids->size(); i++){
      decimalindx = decimalindx + pow(2,i)*indx.at(current_read_ids->at(i));
    }
    guesses_percolumn.push_back(decimalindx);
    
 
    
    ColumnIndexingScheme* current_indexer = next_indexer;
 
    if (column_iterator.has_next()) {
      
      next_column = column_iterator.get_next();
      next_read_ids = extract_read_ids(*next_column);
      next_indexer = new ColumnIndexingScheme(current_indexer,*next_read_ids);
    
      current_indexer->set_next_column(next_indexer);
      indexers.push_back(next_indexer);
    } else {
      
      assert(next_column.get() == 0);
      assert(next_read_ids.get() == 0);
      read_count = column_iterator.get_read_count();
      next_indexer = 0;
     
    }
  
    vector<unsigned int> dp_column(current_indexer->column_size(),0);
    vector<unsigned int>* backtrace_column = 0;
    // if not last column, reserve memory for forward projections column
    if (next_column.get() != 0) {
#ifdef DBW
      cout << i << " : " << endl;
      ++i;
      cout << "allocate current projection / backtrace columns of size : " << current_indexer->forward_projection_size() << endl;
      cout << "forward projection width : " << current_indexer->get_forward_projection_width() << endl << endl;
#endif


      current_projection_column = auto_ptr<vector<unsigned int> >(new vector<unsigned int>(current_indexer->forward_projection_size(), numeric_limits<unsigned int>::max()));
      // NOTE: forward projection size will always be even
#ifdef HALF_TABLE
      backtrace_column = new vector<unsigned int>((current_indexer->forward_projection_size()>>1), numeric_limits<unsigned int>::max());
#else
      backtrace_column = new vector<unsigned int>(current_indexer->forward_projection_size(), numeric_limits<unsigned int>::max());
#endif
    }

    // do the actual compution on current column
    ColumnCostComputer cost_computer(*current_column, all_heterozygous);
  
    auto_ptr<ColumnIndexingIterator> iterator = current_indexer->get_iterator_3(decimalindx);  

    // db
#ifdef DBW
    cout << "previous projection column (costs) :" << endl << endl;
    if(previous_projection_column.get()!=0) {
      output_vector_1_enum(previous_projection_column.get(),current_indexer->get_backward_projection_width());
    }
    cout << endl;

    cout << "row ids : ";
    output_vector_1(current_indexer->get_read_ids());

    cout << " .. column size : " << current_indexer->column_size() << endl;
    
    cout << "forward projection mask : ";
    if(next_column.get()!=0) {
      output_vector_1(current_indexer->get_forward_projection_mask());
      cout << " .. width : " << current_indexer->get_forward_projection_width();
    }
    cout << endl;

    cout << "------------------" << endl;
#endif
    
    while (iterator->has_next_2()) {
      
      int *bit_changed = (int*)malloc(sizeof(int)*30);
      
      for(int i=0; i<30; i++){
        bit_changed[i]=-1;
      }
    
      
      iterator->advance_2(&bit_changed, decimalindx); 
      
      
      
      int bit_flag=0;
      for(int i=0; i<30; i++)
        if(bit_changed[i]!=-1)
          bit_flag=1;
        
      if (bit_flag == 1) {
        
        for(int i=0; i<30; i++)
          if(bit_changed[i]!=-1)
            cost_computer.update_partitioning(bit_changed[i]);
      } else {
        cost_computer.set_partitioning(iterator->get_partition());
        if(next_column.get() == 0) { 
          running_optimal_score_index = iterator->get_index();
        }
      }
      
    
      delete bit_changed;
    
      unsigned int cost = 0;
      if (previous_projection_column.get() != 0) {
        cost += previous_projection_column->at(iterator->get_backward_projection());
      }
     
       
      // db
#ifdef DBW
      cout << iterator->get_backward_projection() << " [" << bit_rep_1(iterator->get_backward_projection(), current_indexer->get_backward_projection_width()) << "] -> " << cost;
#endif

      cost += cost_computer.get_cost();
       
      // db
#ifdef DBW
      cout << " + " << cost_computer.get_cost() << " = " << cost << " -> " << iterator->get_index() << " [" << bit_rep_1(iterator->get_index(), current_indexer->get_read_ids()->size()) << "]";
      if(next_column.get()!=0) {
        cout << " -> " << iterator->get_forward_projection() << " [" << bit_rep_1(iterator->get_forward_projection(), current_indexer->get_forward_projection_width()) << "]";// fpw = " << current_indexer->get_forward_projection_width();
      }
      cout << endl;
#endif
      
      dp_column[iterator->get_index()] = cost;
     
      if (next_column.get() == 0) {
        // update running optimal score index
        if (cost < dp_column[running_optimal_score_index]) {
          running_optimal_score_index = iterator->get_index();
        }
      } else {
        
        unsigned int forward_index = iterator->get_forward_projection();
        if (current_projection_column->at(forward_index) > cost) {
          current_projection_column->at(forward_index) = cost;
#ifdef HALF_TABLE
          forward_index = min(forward_index, iterator->get_forward_dual_projection());
          // ensure that we pass the smaller of the two indexes,
          // because backtrace_column is half the size of
          // current_projection_column (which is symmetric, hence this
          // dual computation)
#endif
          backtrace_column->at(forward_index) = iterator->get_index();
        }
      }
    }
    

    // db
#ifdef DBW
    cout << endl;
#endif

    if(next_column.get() == 0) { 
      // record optimal score
      running_optimal_score = dp_column[running_optimal_score_index];
    }
   
    // add newly computed backtrace_table column
    backtrace_table.push_back(backtrace_column);
     ++n;

    // db
    /*
    for(size_t j=0;j<current_column->size(); ++j) {
      cout << dp_column->at(j) << endl;
    }
    cout << endl;
    */

    // completion percentage output
    /*
    if((double(n)/double(nc)) > pc) {
      cout << int(pc*100.0) << " %" << endl;
      pc += pi;
    }
    */
    
  }

  // store optimal score for table at end of computation
  optimal_score = running_optimal_score;
  optimal_score_index = running_optimal_score_index;
  
  //cout<<"In.....in compute table\n";
}

unsigned int DPTable_whitecodes::get_optimal_score() {
  //if (backtrace_table.empty()) throw runtime_error("Empty backtrace table");
  return optimal_score;
}

auto_ptr<vector<unsigned int> > DPTable_whitecodes::get_index_path() {

  auto_ptr<vector<unsigned int> > index_path = auto_ptr<vector<unsigned int> >(new vector<unsigned int>(indexers.size()));
  unsigned int index = optimal_score_index;
  index_path->at(indexers.size()-1) = index;
  
  for(size_t i = indexers.size()-1; i > 0; --i) {
    int column_guess= guesses_percolumn.at(i);// backtrack through table
   // int column_pernextguess=guesses_pernextcolumn.at(i);
    if(i>0) {
      auto_ptr<ColumnIndexingIterator> iterator = indexers[i]->get_iterator_3(column_guess);
      unsigned int backtrace_index = iterator->index_backward_projection(index);
      index = backtrace_table[i-1]->at(backtrace_index);
     // cout<<"index"<< index;
      index_path->at(i-1) = index;
    }
  }

  return index_path;
}

void DPTable_whitecodes::get_super_reads(ReadSet* output_read_set) {
 // cout<<"In.....in super reads\n";
  assert(output_read_set != 0);

  ColumnIterator column_iterator(*read_set);
  const vector<unsigned int>* positions = column_iterator.get_positions();

  Read* r0 = new Read("superread0", -1);
  Read* r1 = new Read("superread1", -1);
  
  if (backtrace_table.empty()) {
    assert(!column_iterator.has_next());
  } else {
    // run through the file again with the column_iterator
    unsigned int i = 0; // column index
    auto_ptr<vector<unsigned int> > index_path = get_index_path();
    while (column_iterator.has_next()) {
      unsigned int index = index_path->at(i);
      auto_ptr<vector<const Entry *> > column = column_iterator.get_next();
      ColumnCostComputer cost_computer(*column, all_heterozygous);
      cost_computer.set_partitioning(index);

      r0->addVariant(positions->at(i), '?', cost_computer.get_allele(0), cost_computer.get_weight(0));
      r1->addVariant(positions->at(i), '?', cost_computer.get_allele(1), cost_computer.get_weight(1));
      ++i; // next column
    }
  }

  output_read_set->add(r0);
  output_read_set->add(r1);
}

vector<bool>* DPTable_whitecodes::get_optimal_partitioning() {

  auto_ptr<vector<unsigned int> > index_path = get_index_path();

  // db
  /*
  for(size_t i=0; i< index_path.size(); ++i) {
    cout << "index : " << index_path[i] << " " << endl;
    for(size_t j=0; j< indexers[i]->get_read_ids()->size(); ++j) {
      cout << indexers[i]->get_read_ids()->at(j) << endl;
    }
  }
  */

  vector<bool>* partitioning = new vector<bool>(read_count,false);
  for(size_t i=0; i< index_path->size(); ++i) {
    unsigned int mask = 1; // mask to pass over the partitioning (i.e., index)
    for(size_t j=0; j< indexers[i]->get_read_ids()->size(); ++j) {
      unsigned int index = index_path->at(i);
      if((index & mask) == 0) { // id at this index is in p0 (i.e., in the part.)
        partitioning->at(indexers[i]->get_read_ids()->at(j)) = true;
      }
      mask = mask << 1;
    }
  }
  
  return partitioning;
}
