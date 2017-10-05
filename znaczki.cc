/*
 * Authors:
 * Michał Jaroń Mateusz Macias
 * JNP1 ZAD1
 * Please compile with -lboost_regex -std=c++11 params.
*/
#include <iostream>
#include <tuple>
#include <set>
#include <boost/regex.hpp>


/*
 * Generally using namespace is bad practice, but here,
 * in such a small program is justified and makes code more clear.
*/
using namespace std;


/*
 * Predefinied values
*/
const string YEAR_REGEX = "([1-9][0-9][0-9][0-9])";
const string PRICE_REGEX =
    "([0-9]{1,9}|[0-9]{1,9}.[0-9]{1,5}|[0-9]{1,9},[0-9]{1,5})";
const string STAMP_REGEX_PATTERN =
    "\\s*(.*)\\s+" + PRICE_REGEX + "\\s+" + YEAR_REGEX + "\\s+(.+)\\s*";
const string QUERY_REGEX_PATTERN =
    "\\s*+" + YEAR_REGEX + "\\s+" + YEAR_REGEX + "\\s*";

// Technical, just cause we don't want to end output with endl char.
bool endl_necessary_in_cout = false;
bool endl_necessary_in_cerr = false;


/*
 * Types and structures to hold and process data about stamps and queries
*/
// Name, price, year, country, dummy bound
typedef tuple<string, string, int, string, int> stamp;

// Whole query, lower bound, uper_bound
typedef tuple<string, int, int> query;

// Comparator to sort stamps by year
struct stampcomp {
  bool operator() (const stamp& lhs, const stamp& rhs) const
  {

    // In case if year, country are the same, price order is deciding
    float price_lhs, price_rhs;

    string tmp_rhs = get<1>(rhs), tmp_lhs = get<1>(lhs);
    replace(tmp_rhs.begin(), tmp_rhs.end(), ',', '.');
    replace(tmp_lhs.begin(), tmp_lhs.end(), ',', '.');

    //Create number from string
    price_lhs = stof(tmp_lhs);
    price_rhs = stof(tmp_rhs);

    if(get<2>(lhs) != get<2>(rhs)){
        return get<2>(lhs) < get<2>(rhs);

    } else if(get<4>(lhs) != get<4>(rhs)){
        return get<4>(lhs) < get<4>(rhs);

    } else if(get<3>(lhs) != get<3>(rhs)){
        return get<3>(lhs) < get<3>(rhs);

    } else if(price_lhs != price_rhs){
        return price_lhs < price_rhs;

    } else {
        return get<0>(lhs) < get<0>(rhs);
    };
  }
};

// Container to hold data about stamps (already sorted by year)
multiset<stamp, stampcomp> stamp_album;
typedef multiset<stamp, stampcomp>::iterator album_iterator;


// Check if string is valid stamp declaration
bool is_stamp(string str){
    boost::regex stamp_pattern(STAMP_REGEX_PATTERN);
    return boost::regex_match(str, stamp_pattern);
}

// Delete space from last non-space characket to end of string
string erase_trailing_spaces(string s)
{
    return s.erase(s.find_last_not_of(" \n\r\t") + 1);
}

void write_error_data(string s, int line_number);// Just a definition

// From valid stamp declaration produces entity of stamp
stamp decode_stamp(string str, int line){
    boost::regex stamp_pattern(STAMP_REGEX_PATTERN);
    boost::smatch sm;
    boost::regex_match(str, sm, stamp_pattern);

    // Split stamp declaration
    string name = sm[1].str();
    name = erase_trailing_spaces(name);

    string price = sm[2].str();
    price = erase_trailing_spaces(price);

    int year = stoi(sm[3].str());

    string country = sm[4].str();
    country = erase_trailing_spaces(country);

    if(name.length() == 0 || price.length() == 0|| country.length() == 0)
        write_error_data(str, line);

    return make_tuple(name, price, year, country, 0);
}

// Check if string is valid query declaration
bool is_query(string str){
    boost::regex query_pattern(QUERY_REGEX_PATTERN);
    return boost::regex_match(str, query_pattern);
}

// From valid query declaration produces entity of query
query decode_query(string str){
    boost::regex query_pattern(QUERY_REGEX_PATTERN);
    boost::smatch sm;
    boost::regex_match(str, sm, query_pattern);

    // Split query
    string input = sm[0].str();
    int begin = stoi(sm[1].str());
    int end = stoi(sm[2].str());

    return make_tuple(input, begin, end);
}

// Outputs data about stamp in a way specified in task
void write_stamp_data(stamp st){

    if (endl_necessary_in_cout){
        cout<<endl;
    } else {
        endl_necessary_in_cout = true;
    }

    // Prepare components for output
    string name = get<0>(st);
    string price = get<1>(st);
    string year = to_string(get<2>(st));
    string country = get<3>(st);

    string output = year + " " + country
        + " " + price + " " + name;

    cout << output;
}

// Writes info about error in a way specified in task
void write_error_data(string s, int line_number){
    if(endl_necessary_in_cerr){
        cerr<<endl;
    } else {
        endl_necessary_in_cerr = true;
    }
    cerr << "Error in line " << line_number
        << ":" << s;
}

// Produces dummy tuple with specified year. Just for comparing
stamp dummy_stamp(int year, int bound){
    return make_tuple("dummy", "0,0", year, "dummy", bound);
}

// Writes all stamps from given interval
void write_all_from_interval(int s_year, int e_year){
    album_iterator start = stamp_album.lower_bound(dummy_stamp(s_year, -1));
    album_iterator end = stamp_album.upper_bound(dummy_stamp(e_year, 1));
    for(album_iterator it = start; it != end; it++){
        write_stamp_data(*it);
    }
}

/* If stamp descriptions are already done
    and we are now waiting for queries?*/
enum reading_state{
    READING_STAMPS = 0,
    READING_QUERIES = 1,
};


int main(void){
    int line_number = 0;
    string current_line;
    enum reading_state current_state = READING_STAMPS;

    while(getline(cin, current_line)){
        line_number++;

        if(current_state == READING_STAMPS && is_stamp(current_line)){
            stamp st = decode_stamp(current_line, line_number);
            stamp_album.insert(st);
        } else if(is_query(current_line)){
            current_state = READING_QUERIES;// Begin of queries
            query q = decode_query(current_line);
            if(get<1>(q)>get<2>(q)){
                write_error_data(current_line, line_number);
            } else {
                // Answer the question
                write_all_from_interval(get<1>(q), get<2>(q));
            }
        } else {
            write_error_data(current_line, line_number);
        }
    }

    return 0;
}
