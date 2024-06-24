#include <span>
#include <algorithm>
#include <vector>
#include <iostream>
#include <memory>
#include <array>
#include <cassert>
#include <numeric>
#include <typeinfo>
#include <ranges>
#include <string_view>
#include <tuple>
#include <cmath>


constexpr bool DBG = false;
bool start_debug = false;

constexpr int DEPTH = 3;
class PascalTable {
public:
    PascalTable() :table() {
        std::array<int,DEPTH+1> row1 {};
        for (auto &x : row1) {
            x = 1;
        }
        table.push_back(row1);
    }
    void add_row() {
        std::array<int,DEPTH+1> row {};
        auto acc = 1;
        row[0]=1;
        for (auto i=1;i <= DEPTH; ++i) {
            acc += table.back()[i];
            row[i] = acc;
        }
        table.push_back(row);
    }
    std::vector<std::array<int,DEPTH+1>> table;
};
int pascal(int i, int j) {
    assert(i>=0 && j>=0);
    assert(i<=DEPTH);
    static PascalTable table{};
    while (table.table.size()<=j) {table.add_row();}
    return table.table[j][i];
}
constexpr int const_pascal(int i, int j) {
    std::vector<int> vec(i+1);
    std::fill(vec.begin(),vec.end(),1);
    for (int k=0;k!=j;++k) {
        for (int l=0;l!=i;++l) {
            vec[l+1]+=vec[l];
        }
    }
    return vec[i];
}

/*
                // a+1 \\     / a+b \
 pascal(a,b) = ((       )) = (       ) = pascal(b,a)
                \\  b  //     \  b  / 
*/
// number of ordered [x;n] with m choices each: pascal(n,m-1)

void test_pascal() {
    std::cout << "pascal triangle test:\n";
    for (auto i=0;i<3;i++) {
        for (auto j=0;j<10;j++) {
            std::cout << pascal(i,j) << " ";
        }
        std::cout << "\n";
    }
}

int next_simple(const std::span<int> span, const int max) {
    const auto elem = std::find_if(span.begin(),span.end(),[=](int n){return n!=max;});
    if (elem == span.end()) {return 0;}
    std::fill(span.begin(),elem+1,*elem +1);
    return (elem-span.begin())+1;
}

void test_next_simple() {
    std::array buf {0,0,0,0,0};
    std::cout << "next_simple test:";
    do {
        for (auto i:buf) {
            std::cout << i << " ";
        }
        std::cout << "\n";
    } while (next_simple(std::span{buf},4));
}
template<typename MASK>
bool next_masked(const std::span<int> span, const MASK mask_begin, const MASK mask_end, const int last_true) {
    const auto elem = std::find_if(span.begin(),span.end(),[=](int n){return n!=last_true;});
    if (elem == span.end()) {return false;}
    std::fill(span.begin(),elem+1,std::find(mask_begin+*elem+1,mask_end,true)-mask_begin);
    return true;
}

void test_next_masked() {
    std::array buf {1,1,1,1,1};
    std::array mask {false,true,true,false,false,true};
    std::cout << "next_masked test:";
    do {
        for (auto i:buf) {
            std::cout << i << " ";
        }
        std::cout << "\n";
    } while (next_masked(std::span{buf},mask.begin(),mask.end(),5));
}

template<std::size_t L>
int len_of_index(const int index, const int count_true) {
    for (auto i=1; i != L+1; ++i)
        if (index < pascal(i,count_true)-1) return i;
    return 0; // unreachable
}

void test_len_of_index() {
    std::array buf {1,1,1};
    std::array mask {false,true,true,false,false,true};
    std::cout << "len_of_index test:";
    for (auto i=1uz,j=0uz;i<=3;++i) {
        std::ranges::fill(buf,1);
        do {
            assert(len_of_index<3>(j,3) == i);
            j++;
        } while (next_masked(std::span{buf.begin(),i},mask.begin(),mask.end(),5));
    }
}

void print_array(const std::span<const int> array, const int len) {
    std::cout << "[" << ((len==0)?"|":" ");
    for (int i=0;i<array.size();++i) {
        std::cout << array[i] << ((i+1==len)?"|":" ");
    }
    std::cout << "]";
}

int* first_trues_check = nullptr;

template<std::size_t L, typename MASK>
bool increment_section(MASK mask_begin,const MASK mask_end,
                       std::span<int>& cndi_span,
                       const std::span<int> dlbn_span,
                       const int first_true, const int last_true, const int count_true) 
{
    //int acc=0;
    //const int nr_indices = std::ranges::size(std::ranges::take_while_view(cndi_span,
    //    [&](int index) {acc += len_of_index<L>(index, count_true); return acc != dlbn_span.size();}))+1;
    if (DBG) {
        std::cout << "increment_section called.\nmask: [";
        for (auto m=mask_begin;m!=mask_end;++m) {std::cout << *m << " ";}
        std::cout << "]\ncndi_span: ";
        print_array(cndi_span,50);
        std::cout << "\ndlbn_span: ";
        print_array(dlbn_span,50);
        std::cout << "\nfirst_true: "<<first_true<<",last: "<<last_true<<",count: "<<count_true<<"\n";
    }
    //std::cout << ""<<*first_trues_check;
    int i = 0;
    for (int acc=0;acc<dlbn_span.size();++i) 
        acc += len_of_index<L>(cndi_span[i],count_true);
    const int nr_indices = i;
    if (DBG) std::cout << "nr_indices: "<<nr_indices<<"\n";
    
    //std::cout << ""<<*first_trues_check;
    
    if (next_masked(dlbn_span.last(len_of_index<L>(cndi_span[nr_indices-1],count_true)), mask_begin,mask_end,last_true)) {
        ++cndi_span[nr_indices-1];
        if (DBG) std::cout << "could increment last section in place.\n";
        //std::cout << ""<<*first_trues_check;
        //std::cout << "\n";
        return true;
    }
    
    //std::cout << ""<<*first_trues_check;
    if (nr_indices == 1) { 
        
        cndi_span=cndi_span.subspan(nr_indices);
        //std::cout << ""<<*first_trues_check;
        //std::cout << "\n";
        return false;
    }
    
    //std::cout << ""<<*first_trues_check;
    auto dlbn_set_span = dlbn_span.first(dlbn_span.size()-len_of_index<L>(cndi_span[nr_indices-1],count_true));
    int new_nr_indices = nr_indices-1;
    auto last_len = len_of_index<L>(cndi_span[new_nr_indices-1],count_true);
    
    //std::cout << ""<<*first_trues_check;
    if (next_masked(dlbn_set_span.last(last_len), mask_begin, mask_end, last_true)) {
        ++cndi_span[new_nr_indices-1];
        last_len = len_of_index<L>(cndi_span[new_nr_indices-1],count_true);
    } else {
        if (last_len+dlbn_set_span.size()+2 > dlbn_span.size()) {
            std::fill(dlbn_set_span.end()-last_len, dlbn_span.end(),first_true);
            cndi_span[new_nr_indices-1] = pascal(dlbn_span.size()-dlbn_set_span.size()+last_len-1,count_true)-1;
            std::ranges::move(cndi_span.subspan(nr_indices),cndi_span.begin()+new_nr_indices);
            cndi_span = cndi_span.first(cndi_span.size()-1);
            //std::cout << ""<<*first_trues_check;
            //std::cout << "\n";
            return true;
        }
        dlbn_set_span = dlbn_span.first(dlbn_set_span.size()+1);
        std::ranges::fill(dlbn_set_span.last(last_len-1),first_true);
        cndi_span[new_nr_indices-1] = pascal(last_len,count_true)-1;
        if (dlbn_set_span.size() == dlbn_span.size()) {
            std::ranges::move(cndi_span.subspan(nr_indices),cndi_span.begin()+new_nr_indices);
            cndi_span = cndi_span.first(cndi_span.size()-1);
            //std::cout << ""<<*first_trues_check;
            //std::cout << "\n";
            return true;
        }
    }
    //std::cout << "a"<<*first_trues_check;
    assert(last_len != 0);
    auto mod = (dlbn_span.size()-dlbn_set_span.size())%last_len;
    auto end = dlbn_span.end();
    if (mod != 0) {end -= 2*last_len;}
    auto target = dlbn_set_span.end();
    assert(cndi_span.size() >= nr_indices);
    
    
    //std::cout << "a"<<*first_trues_check;
    //std::cout << "\nmoving to "<<(void*)&*(cndi_span.end()+dlbn_span.size())<<"\n";

    
    std::ranges::move_backward(cndi_span.subspan(nr_indices),cndi_span.end());
    //std::cout <<*first_trues_check<<"o";
    
    
    for (;target<end;target+=last_len) {
        std::ranges::copy(dlbn_set_span.last(last_len),target);
        cndi_span[new_nr_indices] = cndi_span[new_nr_indices-1];
        ++new_nr_indices;
    }
    //std::cout << ""<<*first_trues_check;
    if (mod != 0) {
        if (dlbn_span.size()-dlbn_set_span.size()<last_len) {target-=last_len;--new_nr_indices;}
        std::fill(target, end, first_true);
        cndi_span[new_nr_indices] = pascal(last_len+mod-1, count_true)-1;
        ++new_nr_indices;
    }
    //std::cout << ""<<*first_trues_check;
    std::ranges::move(cndi_span.subspan(nr_indices),cndi_span.begin()+new_nr_indices);
    
    cndi_span = std::span {cndi_span.begin(),cndi_span.size()+new_nr_indices-nr_indices};
    //std::cout << ""<<*first_trues_check;
    //std::cout << "\n";
    return true;
}

int get_index(const std::span<const int> sorted_span, const int nr_base_numbers) {
    return pascal(sorted_span.size(),nr_base_numbers-1)-1
    -std::accumulate(sorted_span.begin(),sorted_span.end(),0,[=,n=0](auto acc,auto val) mutable {
        ++n;if (val==nr_base_numbers-1) {return acc;} else {return acc+pascal(n,nr_base_numbers-val-2);}});
}

template<std::size_t L>
int choices(const std::span<int> dlbn_span, const std::span<int> dlbn_sorted_span, int const * cndi_iter, const std::span<int> count_trues, const std::span<int> dlcn_span) {
    int choices = 1;
    if (DBG) {
        std::cout << "\ndlbn_span: ";print_array(dlbn_span,100);std::cout << "\nordered dlbn span: ";print_array(dlbn_sorted_span,100);
        std::cout << "\ncndi_iter: ";print_array(std::span{cndi_iter,dlbn_span.size()},100);
        std::cout << "\ncount_trues: ";print_array(count_trues,100);
    }
    std::vector<std::pair<int,int>> adds{}; 
    for (auto [base_der,free] : dlbn_sorted_span 
    | std::views::chunk_by(std::ranges::equal_to{}) 
    | std::views::transform([](auto chunk){return std::make_pair(*(chunk.begin()),chunk.size());})) {
        auto i=cndi_iter;
        if (DBG) std::cout << "\nfree = "<<free<<", dase_der = "<<base_der;
        for (auto j=0,len=0; j!=dlbn_span.size();j+=len) {
            if (base_der == *std::ranges::max_element(dlbn_span.subspan(j,len))) {
                auto entry = std::find_if(adds.begin(),adds.end(),[x=dlcn_span[j]](auto pair){return pair.first == x;});
                if ( entry != adds.end() ) {++((*entry).second);} else {adds.push_back(std::make_pair(dlcn_span[j],1));}
            }
            len=len_of_index<L>(*(i++),count_trues[j]);
            int n = std::ranges::count(dlbn_span.subspan(j,len), base_der);
            choices *= pascal(free-n,n);
            free -= n;
        }
    }
    for (auto [nr,count] : adds) {
        auto iota = std::views::iota(2,count+1);
        choices /= std::reduce(iota.begin(),iota.end(),1,[](auto x,auto y){return x*y;});
    }
    if (DBG) std::cout << "\n -> choices: "<<choices<<"\n";
    return choices;
}

template<std::size_t L, typename MASKS, typename GBD, typename ACD>
/* MASKS: 
 * function std::pair<MASK begin,MASK end> MASKS(int n: 0..<nr_rows)
 * MASK: iterator over bools,
 * for<int n: 0..<nr_rows >
 * for<int m: 0..<MASKS(n).second-MASKS(n)-first >
 * *(MASKS(n)[m]): bool indicating if combined number n contains derivatives after base number m
 * (we are calculating the 'combining number', 
 *  which has its derivatives in terms of the 'combined numbers',
 *  and the 'base numbers' are the set of all numbers in terms of which the 'combined numbers' 
 *  derivatives are stored.)
 */
void double_combinations_iterate(MASKS masks, const int nr_base_numbers, int nr_rows, GBD get_base_derivative, ACD add_combined_derivative) {
    /*for (int i = 0;i<nr_rows; ++i) {
        for (bool b : masks(i)) {
            std::cout << b<<", ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";*/
    
    std::array<int,L> derivative_levels_combined_numbers {};
    std::array<int,L> derivative_levels_base_numbers {};
    std::array<int,L> combined_number_derivative_indices {};
    int number_combined_number_derivatives = 0;

    std::array<int,L> first_trues {};
    std::array<int,L> last_trues {};
    std::array<int,L> count_trues {};

    int nr_dlcn_changed = 0;

    int base_index = 0;

    for(int number_combined_derivative_levels: std::views::iota(1uz,L+1)) {
        auto print = [&] () {
            std::cout << "derivative_levels_combined_numbers: ";
            print_array(std::span{derivative_levels_combined_numbers},number_combined_derivative_levels);
            std::cout << "\nderivative_levels_base_numbers: ";
            print_array(std::span{derivative_levels_base_numbers},number_combined_derivative_levels);
            std::cout << "\nfirst_trues: ";
            print_array(std::span{first_trues},number_combined_derivative_levels);
            std::cout << "\nlast_trues: ";
            print_array(std::span{last_trues},number_combined_derivative_levels);
            std::cout << "\ncount_trues: ";
            print_array(std::span{count_trues},number_combined_derivative_levels);
            std::cout << "\ncombined_number_derivative_indices: ";
            print_array(std::span{combined_number_derivative_indices},number_combined_number_derivatives);
            std::cout << "\n";            
        };
        std::ranges::fill(derivative_levels_combined_numbers,0);
        nr_dlcn_changed = number_combined_derivative_levels;
        do {
            auto test = derivative_levels_combined_numbers[0];
            auto test_len = number_combined_derivative_levels;
            auto mask_span = masks(derivative_levels_combined_numbers[0]);
            //auto [mask_begin,mask_end] = masks(derivative_levels_combined_numbers[0]);
            auto mask_begin = mask_span.begin();
            auto mask_end = mask_span.end();

            
            auto count_true = std::count(mask_begin, mask_end, true);
            if (count_true == 0) {
                nr_dlcn_changed = next_simple(std::span{derivative_levels_combined_numbers}.first(number_combined_derivative_levels),nr_rows-1);
                continue;
            }
            std::fill(count_trues.begin(),count_trues.begin()+nr_dlcn_changed,count_true);
            int first_true = std::find(mask_begin, mask_end, true) - mask_begin;
            /* std::cout << "filling first "<<nr_dlcn_changed<<" values in ";
            print_array(std::span{first_trues},100);
            std::cout << " with "<<first_true<< " => ";
            */
            std::fill(first_trues.begin(),first_trues.begin()+nr_dlcn_changed,first_true);
            /*print_array(std::span{first_trues},100);
            std::cout << "\n";*/
            //std::fill(last_trues.begin(),last_trues.begin()+nr_dlcn_changed,
            //          std::ranges::find_last(mask_begin, mask_end, true).begin() - mask_begin);
            auto last = mask_end-1;
            while (!*last) {--last;}
            std::fill(last_trues.begin(),last_trues.begin()+nr_dlcn_changed,
                      last - mask_begin);
            
            
            /*print_array(std::span{first_trues},100);*/
            std::ranges::copy(std::span{first_trues}.first(number_combined_derivative_levels),
                              derivative_levels_base_numbers.begin());
            /*std::cout << " => ";
            print_array(std::span{first_trues},100);
            std::cout << "\n\n";*/
            std::ranges::fill(combined_number_derivative_indices,0);
            number_combined_number_derivatives = number_combined_derivative_levels;

            outer: while (true) {
                if (DBG) {print(); std::cout << "\n";}
                const auto dlbn_full_span = std::span{derivative_levels_base_numbers}.first(number_combined_derivative_levels);
                auto sorted_buf = std::array<int,L> {};
                std::ranges::copy(dlbn_full_span, sorted_buf.begin());
                std::span sorted_span {sorted_buf.begin(),dlbn_full_span.size()};
                std::ranges::sort(sorted_span, [](auto a,auto b){return a>b;});
                const int new_index = base_index + get_index(sorted_span, nr_base_numbers);
                const int nr_choices = choices<L>(dlbn_full_span, sorted_span, combined_number_derivative_indices.begin(), std::span{count_trues}, std::span{derivative_levels_combined_numbers});

                double der = 1;
                der *= nr_choices;
                if (DBG) {std::cout << "\n    Add derivatives:\n";}
                for (auto i=0,j=0;i<number_combined_number_derivatives;
                        j+=len_of_index<L>(combined_number_derivative_indices[i++],count_trues[j])) {
                    assert(j < number_combined_derivative_levels);
                    sorted_span[i] = derivative_levels_combined_numbers[j];
                    if (DBG) {std::cout << "index " << combined_number_derivative_indices[i] << " of combined number " << derivative_levels_combined_numbers[j]<<"\n";}
                    der *= get_base_derivative(derivative_levels_combined_numbers[j],combined_number_derivative_indices[i]);
                }
                const int combining_index = get_index(sorted_span.first(number_combined_number_derivatives),nr_rows)
                    +pascal(number_combined_number_derivatives-1,nr_rows)-1;
                if (DBG ) {
                    std::cout << "and index " << combining_index << " of combining number\n";
                    std::cout << "and repetition factor "<<nr_choices<<"\n";
                    std::cout << "to new number at index " << new_index;
                    std::cout << "\n    value: "<< der <<"\n";
                }
                add_combined_derivative(combining_index,new_index,der);
                

                auto sec_begin = derivative_levels_combined_numbers.begin();
                auto cndi_span = std::span{combined_number_derivative_indices}
                    .first(number_combined_number_derivatives);
                while (sec_begin != derivative_levels_combined_numbers.begin()+number_combined_derivative_levels) {
                    auto sec_end = std::find_if(sec_begin,derivative_levels_combined_numbers.begin()+number_combined_derivative_levels,
                                           [sec_b=*sec_begin](int n){return n != sec_b;});
                    //auto [mask_begin,mask_end] = masks(*sec_begin);
                    auto mask_span = masks(*sec_begin);
                    auto mask_begin = mask_span.begin();
                    auto mask_end = mask_span.end();
                    auto section_begin = sec_begin-derivative_levels_combined_numbers.begin();
                    auto dlbn_span = std::span{derivative_levels_base_numbers}
                        .subspan(section_begin,sec_end-sec_begin);
                    if(DBG) print();
                    int ftr = first_trues[section_begin];
                    
                    //std::cout << "first_trues: "<<(void*)(first_trues.begin())<<" - "<<(void*)(first_trues.end())<<"\n";
                    //std::cout << "cndi_span: "<<(void*)(&*cndi_span.begin())<<" - "<<(void*)(&*cndi_span.end())<<"\n";
                    //std::cout << "dlbn_span: "<<(void*)(&*dlbn_span.begin())<<" - "<<(void*)(&*dlbn_span.end())<<"\n";
                    //print_array(std::span{first_trues},100);
                    first_trues_check = &first_trues[1];
                    if (increment_section<L>(mask_begin,mask_end,cndi_span,dlbn_span,ftr,last_trues[section_begin],count_trues[section_begin])) {
                        // increment_section should advance the start of cndi_span 
                        // over the indices belonging to this section if it returns false,
                        // or leave it at the start of this section 
                        // while changing its end if it changes the number of indices used for this section,
                        // when returning true.
                        
                        //std::cout << " => ";
                        //print_array(std::span{first_trues},100);
                        if (DBG) std::cout << "increment_section returned true.\n";

                        int section_begin = sec_begin-derivative_levels_combined_numbers.begin();
                        std::ranges::copy(std::span{first_trues}.first(section_begin),derivative_levels_base_numbers.begin());
                        number_combined_number_derivatives = section_begin + cndi_span.size();
                        std::ranges::move_backward(cndi_span,combined_number_derivative_indices.begin()+number_combined_number_derivatives);
                        std::ranges::fill(std::span{combined_number_derivative_indices}.first(section_begin),0);
                        goto continue_outer;
                    }
                    
                    //std::cout << " => ";
                    //print_array(std::span{first_trues},100);
                    //std::cout << "\n\n";
                    if (DBG) std::cout << "increment_section returned false.\n";
                    sec_begin = sec_end;
                }
                break;
                continue_outer:
                //std::cout << " => ";
                //print_array(std::span{first_trues},100);
                //std::cout << "\n\n";
            }
            nr_dlcn_changed = next_simple(std::span{derivative_levels_combined_numbers}.first(number_combined_derivative_levels),nr_rows-1);
        } while (nr_dlcn_changed != 0); 
        base_index += pascal(number_combined_derivative_levels, nr_base_numbers-1);
    }
}

class Num {
public:
    enum Type {base=254, ptrs=255};
    Type type;
    constexpr Num(Type t) :type{t} {}
};

class BaseNum: public Num {
public:
    constexpr static Num::Type ctype = Num::Type::base;
    double value;
    std::vector<bool> mask;
    int length;
    std::vector<double> derivatives;
    BaseNum(double pvalue,std::vector<bool> pmask, int plength, std::vector<double> pderivatives) :Num{Num::Type::base},value{pvalue},mask{pmask},length{plength},derivatives{pderivatives} {}
    template<typename BASE_SPAN>
    static BaseNum combine_base_nums(double value, BASE_SPAN bases, std::span<double> combining_ders) {
        auto mask_length = (*std::find_if(bases.begin(),bases.end(), [](auto x){return x!=nullptr;}))->mask.size();
        // auto mask_length = (*(bases.begin()))->mask.size();
        std::vector<bool> combined_mask(mask_length);
        for (auto base : bases) {
            if (base!=nullptr)
                std::transform(combined_mask.begin(),combined_mask.end(),base->mask.begin(),combined_mask.begin(),std::logical_or<bool>());
        }
        auto nr_masks = bases.size();
        auto nr_trues = std::count(combined_mask.begin(), combined_mask.end(), true);
        std::vector<std::vector<bool>> masks_buf (nr_masks);
        for (auto i = 0;i != nr_masks;++i) {
            masks_buf[i] = std::vector<bool>(nr_trues);
            auto iter = std::views::zip(combined_mask,std::views::iota(0))
                | std::views::filter([](auto pair){return pair.first;})
                | std::views::transform([&](auto pair){return bases[i]!=nullptr && bases[i]->mask[pair.second];});
            std::ranges::copy(iter, masks_buf[i].begin());
        }
        auto masks = [&](const int n) {
            return std::views::all(masks_buf[n]);
        };
        std::vector<double> derivatives(pascal(DEPTH,nr_trues)-1);
        std::ranges::fill(derivatives,0.);
        auto get_base_derivative = [&](int base_nr, int derivative_nr){
            return (bases[base_nr]==nullptr)?0.:bases[base_nr]->derivatives[derivative_nr];
        };
        auto add_combined_derivative = [&](int combining_index, int combined_index, double derivative_value) {
            derivatives[combined_index] += derivative_value * combining_ders[combining_index];
        };
        double_combinations_iterate<DEPTH>(masks, nr_trues, nr_masks, get_base_derivative, add_combined_derivative);
        return BaseNum{value,combined_mask,(int)nr_trues,derivatives};
        //return BaseNum{Base::combine_base_nums(value, bases | std::views::transform([](BaseNum* base_num){return &(base_num->base);}),combining_ders)};
    }
    BaseNum operator()() {return BaseNum{std::move(value),std::move(mask),std::move(length),std::move(derivatives)};}
    double operator*() const {return value;}
};
/* PTRSNUM should only ever be PtrsNum, have to use templates because AnyNum stores PTRSNUM
and needs it to be a complete type at definition while PtrsNum::operator() returns AnyNum 
and needs it to be a complete type at point of definition
why is c++ not order-independent? */
template<class PTRSNUM>
class AnyNum {
public:
    union {
        BaseNum base;
        PTRSNUM ptrs;
    };
    Num const* num() const {return reinterpret_cast<Num const*>(this);} // may be UB but works, idk if static_cast<Num const*>(&base) is better
    AnyNum(PTRSNUM pptrs) :ptrs{pptrs} {}
    AnyNum(BaseNum pbase) :base{pbase} {}
    double operator*() const {
        if (num()->type == Num::Type::base) {return *base;} else {return *ptrs;}
    }
    AnyNum operator()() {
        if (num()->type == Num::Type::base) {return AnyNum{std::move(base)};} else {return (ptrs.operator())();}
    }
    AnyNum(const AnyNum& base_num) {
        if (num()->type == Num::Type::base) {base = base_num.base;} else {ptrs = base_num.ptrs;}
    }
    AnyNum& operator=(const AnyNum& base_num) {
        if (num()->type == Num::Type::base) {base = base_num.base;} else {ptrs = base_num.ptrs;}
        return *this;
    }
    AnyNum(AnyNum&& base_num) {
        if (num()->type == Num::Type::base) {base = std::move(base_num.base);} else {ptrs = std::move(base_num.ptrs);}
    }
    AnyNum& operator=(AnyNum&& base_num) {
        if (num()->type == Num::Type::base) {base = std::move(base_num.base);} else {ptrs = std::move(base_num.ptrs);}
        return *this;
    }
    Num* operator&() {
        if (num()->type == Num::Type::base) {return &base;} else {return &ptrs;}
    }
    Num const * operator&() const {
        if (num()->type == Num::Type::base) {return &base;} else {return &ptrs;}
    }
    ~AnyNum() {
        if (num()->type == Num::Type::base) {base.BaseNum::~BaseNum();} else {ptrs.PTRSNUM::~PTRSNUM();}
    }
};

class CPtrsNumReader: public Num {
public:
    double value;
    Num* nums;
    double ders;
    std::span<Num*> base_nums() {return std::span{&nums,(std::size_t)Num::type};}
    std::span<double> derivatives() {
        void* end_of_base_nums = &nums+(std::size_t)Num::type;
        std::size_t space = 1024;
        std::align(alignof(CPtrsNumReader),sizeof(double),end_of_base_nums,space);
        return std::span{(double*)end_of_base_nums,(std::size_t)pascal(DEPTH,(int)Num::type)-1};
    }
};

class PtrsNum: public Num {
public:
    constexpr static Num::Type ctype = Num::Type::ptrs;
    double value;
    std::vector<Num*> base_nums;
    std::vector<double> derivatives;
    PtrsNum(double pvalue, std::vector<Num*> pbase_nums, std::vector<double> pderivatives) :Num{Num::Type::ptrs},value{pvalue},base_nums{pbase_nums},derivatives{pderivatives} {}
    double operator*() const {return value;}
    AnyNum<PtrsNum> operator()() {
        if (std::ranges::all_of(base_nums,[](auto x) {return x->type==Num::Type::base;})) {
            return AnyNum<PtrsNum>{BaseNum::combine_base_nums(value, base_nums | std::views::transform([](Num* num){
                return (static_cast<BaseNum*>(num));
        }), std::span{derivatives})};
        } else {
            std::vector<Num*> bases {};
            for (auto num : base_nums) {
                if (num->type==Num::Type::base) {
                    if (std::find(bases.begin(),bases.end(),num) == bases.end()) {
                        bases.push_back(num);
                    }
                } else if (num->type==Num::Type::ptrs){
                    for (auto subnum : static_cast<PtrsNum*>(num)->base_nums) {
                        if (std::find(bases.begin(),bases.end(),subnum) == bases.end()) {
                            bases.push_back(subnum);
                        }
                    }
                } else {
                    for (auto subnum : static_cast<CPtrsNumReader*>(num)->base_nums()) {
                        if (std::find(bases.begin(),bases.end(),subnum) == bases.end()) {
                            bases.push_back(subnum);
                        }
                    }
                }
            }
            auto masks = [&](const int n) {
                auto iter = bases | std::views::transform([&,n](auto base_num){
                    if (base_nums[n]->type==Num::Type::base) {
                        return base_nums[n] == base_num;
                    } else if (base_nums[n]->type==Num::Type::ptrs) {
                        auto base_numbers = &(static_cast<PtrsNum*>(base_nums[n])->base_nums);
                        return std::find(base_numbers->begin(),base_numbers->end(),base_num) != base_numbers->end();
                    } else {
                        auto base_numbers = static_cast<CPtrsNumReader*>(base_nums[n])->base_nums();
                        return std::find(base_numbers.begin(),base_numbers.end(),base_num) != base_numbers.end();
                    }
                });
                return iter;
            };
            std::vector<double> new_derivatives(pascal(DEPTH,bases.size())-1);
            auto get_base_derivative = [&](int base_nr, int derivative_nr){
                if (base_nums[base_nr]->type==Num::Type::base) {
                    return (derivative_nr==0)?1.:0.;
                } else if (base_nums[base_nr]->type==Num::Type::ptrs) {
                    return static_cast<PtrsNum*>(base_nums[base_nr])->derivatives[derivative_nr];
                } else {
                    auto result = static_cast<CPtrsNumReader*>(base_nums[base_nr])->derivatives();
                    return result[derivative_nr];
                }
            };
            auto add_combined_derivative = [&](int combining_index, int combined_index, double derivative_value) {
                if (DBG) std::cout << "combining_index: "<<combining_index<<" , combining derivative: "<<derivatives[combined_index]<<" , derivative_value: "<<derivative_value<<" , combined_index: "<<combined_index<<"\n";
                new_derivatives[combined_index] += derivative_value * derivatives[combining_index];
            };
            if (DBG) std::cout << "bases size: " << bases.size() << ", base_nums size: " << base_nums.size() << "\n";
            double_combinations_iterate<DEPTH>(masks, bases.size(), base_nums.size(), get_base_derivative, add_combined_derivative);
            return AnyNum<PtrsNum>{PtrsNum{value, bases, new_derivatives} };
        }
    }
};

std::vector<BaseNum> make_parameters(std::span<double> span) {
    std::vector<BaseNum> vec {};
    for (auto [x,i] : std::views::zip(span, std::views::iota(0))) {
        std::vector<bool> mask(span.size());
        std::ranges::fill(mask,false);
        mask[i] = true;
        std::vector<double> derivatives(DEPTH);
        std::ranges::fill(derivatives,0.);
        derivatives[0] = 1.;
        vec.push_back(BaseNum{x,mask, 1, derivatives});
    }
    return vec;
}


/* All BaseNums -> BaseNum
 * Only BaseNums and ConstNums, not all BaseNums -> ConstNum
 * Only BaseNums and AnyNums, not all BaseNums -> AnyNum
 * Any other -> PtrsNum
 * prev result \ additional Base                Const<B...>             Any     Ptrs    
 * BaseNum                  Base                Const<B...,prev...]>    Any     Ptrs
 * ConstNum<A...>           Const<Base,A...>    Const<B...,A...>        Ptrs    Ptrs
 * AnyNum                   Any                 Ptrs                    Any     Ptrs
 * PtrsNum                  Ptrs                Ptrs                    Ptrs    Ptrs
 */

template<typename ... T>
struct CPNum {using eval_type = PtrsNum;};

template<typename ... T> 
class CPtrsNum;

template<typename ... T>
CPNum<T...>::eval_type eval_impl(CPtrsNum<T...>& num);

/*template<std::size_t len>
class CPtrsNumInner: public Num {
public:
    double value;
    std::array<Num*,len> base_nums;
    std::array<double,const_pascal(DEPTH,len)-1> derivatives;
    constexpr CPtrsNumInner(double pvalue,std::array<Num*,len> pbasenums, std::array<double,const_pascal(DEPTH,len)-1> pders) :Num{(Num::Type)len},value{pvalue},base_nums{pbasenums},derivatives{pders} {}
};*/



template<typename ... T> 
class CPtrsNum: public Num /*CPtrsNumInner<sizeof...(T)>*/ {
public:
    constexpr static std::size_t size {sizeof...(T)};
    constexpr static bool is_cptrsnum = true;
    template<typename ... U> using add_front = CPtrsNum<U...,T...>;

    double value;
    std::array<Num*,sizeof...(T)> base_nums;
    std::array<double,const_pascal(DEPTH,sizeof...(T))-1> derivatives;
    //CPtrsNumInner<sizeof...(T)> inner;
    constexpr CPtrsNum(const int constexpr_overload, double pvalue,std::array<Num*,sizeof...(T)> pbasenums, std::array<double,const_pascal(DEPTH,sizeof...(T))-1> pders) 
        :Num{(Num::Type)sizeof...(T)},value{pvalue},base_nums{pbasenums},derivatives{pders} {}
    CPtrsNum(double pvalue,std::array<Num*,sizeof...(T)> pbasenums, std::array<double,const_pascal(DEPTH,sizeof...(T))-1> pders) 
        :Num{(Num::Type)sizeof...(T)},value{pvalue},base_nums{pbasenums},derivatives{pders} {
            double* first_der_1 = derivatives.begin();
            double* first_der_2 = &(((CPtrsNumReader*)this)->derivatives().front());
            assert(first_der_1==first_der_2);
        }
    double operator*() const {return value;}
    CPNum<T...>::eval_type operator()() {return eval_impl<T...>(*this);};
};

template<typename ... T> requires std::same_as<typename CPNum<T ...>::eval_type,BaseNum>
struct CPNum<BaseNum,T...> {using eval_type = BaseNum;};

template<typename ... T> requires std::same_as<typename CPNum<T ...>::eval_type,BaseNum> || std::same_as<typename CPNum<T ...>::eval_type,AnyNum<PtrsNum>>
struct CPNum<AnyNum<PtrsNum>,T ...> {using eval_type = AnyNum<PtrsNum>;};
template<typename ... T> requires std::same_as<typename CPNum<T ...>::eval_type,AnyNum<PtrsNum>>
struct CPNum<BaseNum,T...> {using eval_type = AnyNum<PtrsNum>;};

template<typename ... T, typename ... B> requires std::same_as<typename CPNum<T ...>::eval_type,BaseNum>
struct CPNum<CPtrsNum<B...>,T...> {using eval_type = CPtrsNum<B...,T...>;};
template<typename ... T> requires CPNum<T...>::eval_type::is_cptrsnum
struct CPNum<BaseNum,T...> {using eval_type = CPNum<T...>::eval_type::template add_front<BaseNum>;};
template<typename ... T, typename ... B> requires CPNum<T...>::eval_type::is_cptrsnum
struct CPNum<CPtrsNum<B...>,T...> {using eval_type = CPNum<T...>::eval_type::template add_front<B...>;};

template<typename T>
struct CPNum<T> {using eval_type = T;};


template<typename ... T> requires std::same_as<typename CPNum<T...>::eval_type,BaseNum>
CPNum<T...>::eval_type eval_impl(CPtrsNum<T...>& num) { // works
    auto base_span = num.base_nums /* | std::views::filter([](Num* x){return x!=nullptr;})*/
        | std::views::transform([](Num* n){return static_cast<BaseNum*>(n);});
    return BaseNum::combine_base_nums(num.value, base_span, std::span{num.derivatives});
}

template<typename TUPLE,typename T>
constexpr std::size_t tuple_find_type = std::tuple_size_v<TUPLE>;
template<typename ... U,typename V,typename T>
constexpr std::size_t tuple_find_type<std::tuple<V,U...>,T> = (std::is_same_v<V,T>)? 0 : tuple_find_type<std::tuple<U...>,T>+1;

template<typename T> struct empty{using type = T;};
template<std::size_t i> struct empty_idx {};
template<typename TYPE_TUPLE, typename TYPES_TUPLE, typename F_TUPLE, typename CONTAINER> 
    requires (std::tuple_size_v<TYPE_TUPLE>+1 == std::tuple_size_v<F_TUPLE>)
void do_for(F_TUPLE f_tuple, CONTAINER c) {
    /*
    for (auto [type,elem] : zip(TYPES_TUPLE,c)) {
        for (auto [test_type,f] : zip(TYPE_TUPLE,f_tuple[..-1]))
            if (type == test_type) { f(elem); break; }
        f_tuple[-1](elem, empty<type>{});
    }
    */
    auto c_iter = c.begin();
    auto check = [&]<std::size_t i>(F_TUPLE& pf, empty_idx<i> empty_i) {
        constexpr auto j = tuple_find_type<TYPE_TUPLE,std::tuple_element_t<i,TYPES_TUPLE>>;
        if constexpr (j < std::tuple_size_v<TYPE_TUPLE>) {
            std::get<j>(pf)(*c_iter);
        } else {
            std::get<j>(pf)(*c_iter,empty<std::tuple_element_t<i,TYPES_TUPLE>>{});
        }
        ++c_iter;
    };
    [&]<std::size_t ... is>(std::index_sequence<is...> seq_i) {
        ((void)(check(f_tuple,empty_idx<is>{})),...);
    }(std::make_index_sequence<std::tuple_size_v<TYPES_TUPLE>>{});
}

void test_do_either() {
    std::array<int,5> arr{0,0,0,0,0};
    do_for<std::tuple<int>, std::tuple<int,double,int,double,double>>(std::make_tuple([](int x){std::cout << "i,";},[]<typename T>(int x,empty<T> e){
        std::cout<<"d,";
        assert((bool)(std::is_same_v<T,double>));
    }),std::span{arr});
    std::cout << "\n";
}

template<typename ... T> requires std::same_as<typename CPNum<T...>::eval_type,AnyNum<PtrsNum>>
CPNum<T...>::eval_type eval_impl(CPtrsNum<T...>& num) { // works
    bool all_basenums = true;
    do_for<std::tuple<AnyNum<PtrsNum>>,std::tuple<T...>>(std::make_tuple([&](Num* any_num){
        if (any_num != nullptr && any_num->type == Num::Type::ptrs) all_basenums = false;
    },[](Num* x,auto e){}), std::span{num.base_nums});
    if (all_basenums) { // this branch works
        return AnyNum<PtrsNum>{BaseNum::combine_base_nums(num.value, num.base_nums /*| std::views::take_while([](Num* x){return x!=nullptr;})*/
            | std::views::transform([](Num* n){return static_cast<BaseNum*>(n);}), std::span{num.derivatives})};
    } else { // this branch also works
        std::vector<Num*> bases {};
        auto include_num = [&](Num* n){if (std::find(bases.begin(),bases.end(),n) == bases.end()) bases.push_back(n);};
        do_for<std::tuple<BaseNum>,std::tuple<T...>>( std::make_tuple([&](Num* base_num){include_num(base_num);} , [&](Num* any_num, auto e){
            if (any_num==nullptr) return;
            if (any_num->type == Num::Type::base) {include_num(any_num);} 
            else { for (Num* n : static_cast<PtrsNum*>(any_num)->base_nums) include_num(n); }
        }),std::span{num.base_nums});
        auto masks = [&](const int n) {
            return bases | std::views::transform([&,n](auto base_num){
                if (num.base_nums[n]->type==Num::Type::base) {return num.base_nums[n] == base_num;} 
                else {auto base_numbers = &(static_cast<PtrsNum*>(num.base_nums[n])->base_nums);
                    return std::find(base_numbers->begin(),base_numbers->end(),base_num) != base_numbers->end();}
            });
        };
        auto get_base_derivative = [&](int base_nr, int derivative_nr){
            if (num.base_nums[base_nr]->type==Num::Type::base) {return (derivative_nr==0)?1.:0.;} 
            else {return static_cast<PtrsNum*>(num.base_nums[base_nr])->derivatives[derivative_nr];}
        };
        std::vector<double> derivatives(pascal(DEPTH,bases.size())-1);
        std::fill(derivatives.begin(),derivatives.end(),0.);
        auto add_combined_derivative = [&](int combining_index, int combined_index, double derivative_value) {
            derivatives[combined_index] += derivative_value * num.derivatives[combining_index];
        };
        double_combinations_iterate<DEPTH>(masks, bases.size(), num.base_nums.size(), get_base_derivative, add_combined_derivative);
        return AnyNum<PtrsNum>{PtrsNum{num.value, bases, derivatives} };
    }
}

template<typename TUPLE, std::size_t i>
struct tuple_set_insert {using type = TUPLE;};
template<typename ...T, std::size_t i> requires (tuple_find_type<std::tuple<T...>,empty_idx<i>> == sizeof...(T))
struct tuple_set_insert<std::tuple<T...>,i> {using type = std::tuple<T...,empty_idx<i>>;};

template<typename ... T>
struct cptrsnum_len_tuple {using type = std::tuple<>;};
template<typename ... T, typename U> requires (tuple_find_type<std::tuple<BaseNum,PtrsNum,AnyNum<PtrsNum>>,U>!=3)
struct cptrsnum_len_tuple<U,T...> {using type = cptrsnum_len_tuple<T...>::type;};
template<typename ... T, typename ... U>
struct cptrsnum_len_tuple<CPtrsNum<U...>,T...> {using type = tuple_set_insert<cptrsnum_len_tuple<T...>,sizeof...(U)>;};

template<std::size_t ... i>
struct empty_idx_tuple {using type = std::tuple<>;};
template<std::size_t i, std::size_t ... rest>
struct empty_idx_tuple<i, rest...> {using type = tuple_set_insert<empty_idx_tuple<rest...>,i>::type;};

template<typename T>
constexpr std::array<std::size_t,std::tuple_size_v<T>> tuple_to_array {};
template<std::size_t ... i>
constexpr std::array<std::size_t, sizeof...(i)> tuple_to_array<empty_idx_tuple<i...>> {i...};

template<typename ... T> requires std::same_as<typename CPNum<T...>::eval_type,PtrsNum>
CPNum<T...>::eval_type eval_impl(CPtrsNum<T...>& num) {
    std::vector<Num*> bases {};
    auto include_num = [&](Num* n){if (n!=nullptr && std::find(bases.begin(),bases.end(),n) == bases.end()) bases.push_back(n);};
    //start_debug = true;
    do_for<std::tuple<BaseNum,PtrsNum,AnyNum<PtrsNum>>,std::tuple<T...>>(std::make_tuple([&](Num* base_num){
        include_num(base_num);
    },[&](Num* ptrs_num){
        for (Num* n : static_cast<PtrsNum*>(ptrs_num)->base_nums) 
            include_num(n);
    },[&](Num* any_num){
        if (any_num->type == Num::Type::base) {
            include_num(any_num);
        }
        else {
            for (Num* n : static_cast<PtrsNum*>(any_num)->base_nums) 
                include_num(n);}
    },[&]<typename U>(Num* c_ptrs_num, empty<U> e){
        for (Num* n : static_cast<CPtrsNumReader*>(c_ptrs_num)->base_nums()) 
            include_num(n);
    }),std::span{num.base_nums} /*| std::views::filter([](Num* x){return x!=nullptr;})*/);
    auto masks = [&](const int n) {
        return bases | std::views::transform([&,n](auto base_num){
            if (base_num == nullptr || num.base_nums[n]==nullptr) return false;
            if (num.base_nums[n]->type==Num::Type::base) {return num.base_nums[n] == base_num;} 
            else if (num.base_nums[n]->type==Num::Type::ptrs) {
                auto base_numbers = &(static_cast<PtrsNum*>(num.base_nums[n])->base_nums);
                return std::find(base_numbers->begin(),base_numbers->end(),base_num) != base_numbers->end();}
            else {
                /*bool result = false;
                do_for<std::tuple<>,cptrsnum_len_tuple<T...>>([&]<std::size_t i>(int zero, empty<empty_idx<i>>){ if (num.base_nums[n]->type==(Num::Type)i) {
                    auto base_numbers = &(static_cast<CPtrsNumInner<i>*>(num.base_nums[n])->base_nums);
                    result = std::find(base_numbers->begin(),base_numbers->end(),base_num) != base_numbers->end();
                }},std::views::repeat(0));*/
                auto base_numbers = static_cast<CPtrsNumReader*>(num.base_nums[n])->base_nums();
                return std::find(base_numbers.begin(),base_numbers.end(),base_num) != base_numbers.end();
            }
        });
    };
    auto get_base_derivative = [&](int base_nr, int derivative_nr){
        if (num.base_nums[base_nr] == nullptr) return 0.;
        auto num_type = num.base_nums[base_nr]->type;
        if (num_type==Num::Type::base) {return (derivative_nr==0)?1.:0.;} 
        else if (num_type==Num::Type::ptrs){return static_cast<PtrsNum*>(num.base_nums[base_nr])->derivatives[derivative_nr];}
        else {
            /*double result {};
            do_for<std::tuple<>,cptrsnum_len_tuple<T...>>([&]<std::size_t i>(int zero, empty<empty_idx<i>>){ if (num_type==(Num::Type)i) {
                result = static_cast<CPtrsNumInner<i>*>(num.base_nums[num_type])->derivatives[derivative_nr];
            }},std::views::repeat(0));*/
            return static_cast<CPtrsNumReader*>(num.base_nums[base_nr])->derivatives()[derivative_nr];
        }
    };
    std::vector<double> derivatives(pascal(DEPTH,bases.size())-1);
    std::fill(derivatives.begin(),derivatives.end(),0.);
    auto add_combined_derivative = [&](int combining_index, int combined_index, double derivative_value) {
        derivatives[combined_index] += derivative_value * num.derivatives[combining_index];
    };
    double_combinations_iterate<DEPTH>(masks, bases.size(), num.base_nums.size(), get_base_derivative, add_combined_derivative);
    return PtrsNum{num.value, bases, derivatives} ;
}
CPtrsNum<> CONSTANT_ZERO {1,0.,std::array<Num*,0>{},std::array<double,0>{}};

template<typename ... T> requires CPNum<T...>::eval_type::is_cptrsnum
CPNum<T...>::eval_type eval_impl(CPtrsNum<T...>& num) {
    std::array<Num*, CPNum<T...>::eval_type::size> bases {};
    std::fill(bases.begin(),bases.end(),nullptr);
    std::size_t next = 0;
    auto include_num = [&](Num* n){
        if (std::find(bases.begin(),bases.begin()+next,n) == bases.begin()+next) {
            bases[next++] = n;
        }
    };
    do_for<std::tuple<BaseNum>,std::tuple<T...>>(std::make_tuple([&](Num* base_num){
        include_num(base_num);
    },[&]<typename U>(Num* c_ptrs_num, empty<U> e){
        for (Num* n : static_cast<U*>(c_ptrs_num)->base_nums) include_num(n);
    }),std::span{num.base_nums});
    auto masks = [&num,&bases](const int n) {
        auto base_n = num.base_nums[n];
        if (base_n==nullptr) {base_n = &CONSTANT_ZERO;}
        auto num_ptr = &num;
        return bases | std::views::transform([base_n,n](auto base_num){
            if (base_num == nullptr) return false;
            if (base_n->type==Num::Type::base) {return base_n == base_num;} 
            else {
                /*bool result = false;
                do_for<std::tuple<>,cptrsnum_len_tuple<T...>>([&]<std::size_t i>(int zero, empty<empty_idx<i>>){ if (base_n->type==(Num::Type)i) {
                    auto base_numbers = &(static_cast<CPtrsNumInner<i>*>(base_n)->base_nums);
                    result = std::find(base_numbers->begin(),base_numbers->end(),base_num) != base_numbers->end();
                }},std::views::repeat(0));*/
                auto base_numbers = static_cast<CPtrsNumReader*>(base_n)->base_nums();
                return std::find(base_numbers.begin(),base_numbers.end(),base_num) != base_numbers.end();
            }
        });
    };
    auto get_base_derivative = [&](int base_nr, int derivative_nr){
        if (base_nr >= num.base_nums.size()) return 0.;
        if (num.base_nums[base_nr] == nullptr) return 0.;
        auto num_type = num.base_nums[base_nr]->type;
        if (num_type==Num::Type::base) {return (derivative_nr==0)?1.:0.;}
        else {
            /*double result {};
            do_for<std::tuple<>,cptrsnum_len_tuple<T...>>([&]<std::size_t i>(int zero, empty<empty_idx<i>>){ if (num_type==(Num::Type)i) {
                result = static_cast<CPtrsNumInner<i>*>(num.base_nums[num_type])->derivatives[derivative_nr];
            }},std::views::repeat(0));*/
            return static_cast<CPtrsNumReader*>(num.base_nums[base_nr])->derivatives()[derivative_nr];
        }
    };
    std::array<double,const_pascal(DEPTH,CPNum<T...>::eval_type::size)-1> derivatives{};
    std::fill(derivatives.begin(),derivatives.end(),0.);
    auto add_combined_derivative = [&](int combining_index, int combined_index, double derivative_value) {
        derivatives[combined_index] += derivative_value * num.derivatives[combining_index];
    };
    double_combinations_iterate<DEPTH>(masks, bases.size(), num.base_nums.size(), get_base_derivative, add_combined_derivative);
    //double_combinations_iterate<DEPTH>()
    return typename CPNum<T...>::eval_type{num.value, bases, derivatives};
}



template<typename T> struct IsNumType : std::false_type {};
template<> struct IsNumType<BaseNum> : std::true_type {};
template<> struct IsNumType<PtrsNum> : std::true_type {};
template<> struct IsNumType<AnyNum<PtrsNum>> : std::true_type {};
template<typename ... T> struct IsNumType<CPtrsNum<T...>> : std::true_type {};
template<typename T> concept NumType = IsNumType<T>::value;

template<typename T> concept NoBaseNum = !std::same_as<T,BaseNum> && !std::same_as<T,AnyNum<PtrsNum>>;
template<typename T, typename U> concept AllowRL = NoBaseNum<T> || std::same_as<T,BaseNum> && std::same_as<U,BaseNum>;
template<typename T, typename U> concept AllowRR = NoBaseNum<T> && NoBaseNum<U> || std::same_as<T,BaseNum> && std::same_as<U,BaseNum>;
template<typename T, typename U> concept AllowAL = std::same_as<T,typename CPNum<T,U>::eval_type>;
template<typename T, typename U> concept AllowAR = AllowAL<T,U> && NoBaseNum<U>;
/*template<NumType T, NumType U>
PtrsNum operator+(T& t, U& u) {
    std::vector<Num*> base_nums {static_cast<Num*>(&t),static_cast<Num*>(&u)};
    std::vector<double> derivatives (pascal(DEPTH,2)-1);
    std::ranges::fill(derivatives,0.);
    derivatives[0] = 1.;
    derivatives[1] = 1.;
    return PtrsNum{*t+*u, base_nums, derivatives};
}*/
template<NumType T, typename OP>
CPtrsNum<T> impl_unary(T& t, OP op) {
    std::array<Num*,1> base_nums {static_cast<Num*>(&t)};
    std::array<double,DEPTH> derivatives{};
    std::ranges::fill(derivatives,0.);
    double value = op(std::span{derivatives},*t);
    return CPtrsNum<T>{value,base_nums,derivatives};
}
template<NumType T, NumType U, typename OP>
CPtrsNum<T,U> impl_binary(T& t, U& u,OP op) {
    std::array<Num*,2> base_nums {static_cast<Num*>(&t),static_cast<Num*>(&u)};
    std::array<double, const_pascal(DEPTH,2)-1> derivatives {};
    std::ranges::fill(derivatives,0.);
    double value = op(base_nums,std::span{derivatives},*t,*u,std::is_same_v<T,U> && (Num*)&t==(Num*)&u);
    return CPtrsNum<T,U>{value,base_nums,derivatives};
}
double plus(std::array<Num*,2>& base_nums, std::span<double> derivatives, double t, double u, bool same) {
    if (same) {
        base_nums[1] = nullptr;
        derivatives[0] = 2.;
    } else {
        derivatives[0] = 1.;
        derivatives[1] = 1.;
    }
    return t+u;
}
template<NumType T, NumType U> auto operator+(T& t, U& u) {return impl_binary(t,u,plus);}
template<NumType T, NumType U> requires AllowRL<T,U> auto operator+(T&& t, U& u) {return impl_binary(t,u,plus)();}
template<NumType T, NumType U> requires AllowRL<U,T> auto operator+(T& t, U&& u) {return impl_binary(t,u,plus)();}
template<NumType T, NumType U> requires AllowRR<T,U> auto operator+(T&& t, U&& u) {return impl_binary(t,u,plus)();}
template<NumType T, NumType U> requires AllowAL<T,U> T& operator+=(T& t, U& u) {return t = impl_binary(t,u,plus)();}
template<NumType T, NumType U> requires AllowAR<T,U> T& operator+=(T& t, U&& u) {return t = impl_binary(t,u,plus)();}

/*template<NumType T, NumType U>
PtrsNum operator-(T& t, U& u) {
    std::vector<Num*> base_nums {static_cast<Num*>(&t),static_cast<Num*>(&u)};
    std::vector<double> derivatives (pascal(DEPTH,2)-1);
    std::ranges::fill(derivatives,0.);
    derivatives[0] = 1.;
    derivatives[1] = -1.;
    return PtrsNum{*t-*u, base_nums, derivatives};
}*/
double minus(std::array<Num*,2>& base_nums, std::span<double> derivatives, double tv, double uv, bool same) {
    if (same) {
        base_nums = {nullptr,nullptr};
    } else {
        derivatives[0] = 1.;
        derivatives[1] = -1.;
    }
    return tv-uv;
}
template<NumType T, NumType U> auto operator-(T& t, U& u) {return impl_binary(t,u,minus);}
template<NumType T, NumType U> requires AllowRL<T,U> auto operator-(T&& t, U& u) {return impl_binary(t,u,minus)();}
template<NumType T, NumType U> requires AllowRL<U,T> auto operator-(T& t, U&& u) {return impl_binary(t,u,minus)();}
template<NumType T, NumType U> requires AllowRR<T,U> auto operator-(T&& t, U&& u) {return impl_binary(t,u,minus)();}
template<NumType T, NumType U> requires AllowAL<T,U> T& operator-=(T& t, U& u) {return t = impl_binary(t,u,minus)();}
template<NumType T, NumType U> requires AllowAR<T,U> T& operator-=(T& t, U&& u) {return t = impl_binary(t,u,minus)();}
double uminus(std::span<double> derivatives, double t) {
    derivatives[0] = -1.;
    return -t;
}
template<NumType T> auto operator-(T& t) {return impl_unary(t,uminus);}
template<NumType T> auto operator-(T&& t) {return impl_unary(t,uminus)();}


template<NumType T, NumType U>
PtrsNum operator*(T& t, U& u) {
    std::vector<Num*> base_nums {static_cast<Num*>(&t),static_cast<Num*>(&u)};
    std::vector<double> derivatives (pascal(DEPTH,2)-1);
    std::ranges::fill(derivatives,0.);
    derivatives[0] = *u;
    derivatives[1] = *t;
    derivatives[3] = 1.;
    return PtrsNum{(*t)*(*u), base_nums, derivatives};
}
template<double y, NumType T> 
PtrsNum pow(T& x) {
    std::vector<Num*> base_nums {static_cast<Num*>(&x)};
    std::vector<double> derivatives (DEPTH);
    double factor = 1.;
    for (int i=0;i!=DEPTH;++i) {
        factor *= y-i;
        derivatives[i] = factor * std::pow(*x,y-i-1);
    }
    return PtrsNum{std::pow(*x,y),base_nums,derivatives};
}


template<NumType T>
void print_base_num(const T& pnum, const char* name) {
    assert(reinterpret_cast<Num const*>(&pnum)->type == Num::Type::base);
    auto num = reinterpret_cast<BaseNum const*>(&pnum);
    std::cout << name << " = " << **num << " :\n";
    auto i=0;
    const int first_true = std::find(num->mask.begin(),num->mask.end(),true)
        -num->mask.begin();
    const int last_true = num->mask.rend() 
        -std::find(num->mask.rbegin(),num->mask.rend(),true)-1;
    for (auto depth=0;depth!=DEPTH;++depth) {
        std::vector<int> buf (depth+1);
        std::fill(buf.begin(),buf.end(),first_true);
        do {
            std::cout << "d^"<<(depth+1)<<" /";
            for (auto index : buf | std::views::reverse) {
                std::cout << " dp"<<index;
            }
            std::cout << " = " << num->derivatives[i++] << "\n";
        } while(next_masked(std::span{buf}, num->mask.begin(), num->mask.end(),last_true));
    }

}

int main() {
    // test_do_either();
    // CPNum<BaseNum,CPtrsNum<BaseNum,BaseNum>>::f();
    //CPtrsNum<BaseNum,BaseNum>::is_cptrsnum::value;
    //  CPNum<CPtrsNum<BaseNum,BaseNum>,CPtrsNum<BaseNum,BaseNum>>::eval_type::print;
    // CPtrsNum<BaseNum,BaseNum>::add_front<BaseNum> t;
    // t.f();
    //assert(test);
    std::array<double,2> vals {5.,2.};
    auto p = make_parameters(std::span{vals});
    auto ptrs = (p[0] * p[1]);
    auto any_base = (p[0]*p[1])();
    auto any_ptrs = (ptrs * p[0])();
    auto cptrs = (p[0]+p[1]);
    //std::array<Num*,4> pointers {&ptrs,&any_base,&any_ptrs,&cptrs};
    //std::cout << "ptrs: "<<(void*)&ptrs<<"\n";
    //std::cout << "any_base: "<<(void*)&any_base<<"\n";
    //std::cout << "any_ptrs: "<<(void*)&any_ptrs<<"\n";
    //std::cout << "cptrs: "<<(void*)&cptrs<<"\n";
    auto s1 = cptrs+p[0];
    auto s2 = any_ptrs + any_base;
    auto s3 = ptrs+p[0];
    //start_debug = true;
    auto sum_part = (s1+s2)();
    auto sum = (sum_part + s3)();
    //auto y = (x + p[0]) ()(); // AnyNum::ptrs
    //auto z = (y+p[0]) ()();
    //auto y = pow<4.>(p[1]);
    //auto z = y();
    //auto x1 = (x*p[0]);
    //auto y1 = (x+p[0]);
    //auto y = y1();
    //auto z = y();
    // a*a*b+a
    auto sum1 = sum();
    auto sum2 = sum1();
    auto sum3 = sum2();
    // 3a+b+2a*b+a*a*b
    auto y = -(p[0]+p[1])()+p[0];
    auto z = p[0]*p[1];
    z += p[0]+p[1];
    print_base_num(sum3()()()()()(),"x");
}
