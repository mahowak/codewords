//  Created by Kyle Mahowald on 4/19/16.
//  Copyright © 2016 Kyle Mahowald. All rights reserved.
//

#ifndef helpers_h
#define helpers_h
#include <vector>
#include <fstream>
#include <sstream>
#include <cmath>
#include <set>

using namespace std;


vector< double> normalize_vector(const vector <double > v) {
    vector<double> normalized_vector;
    double s = 0;
    double sd_sum = 0;
    for (int i = 0; i < v.size(); i ++) {
        s += v[i];
    }
    double avg = s/v.size();
    for (int j = 0; j < v.size(); j ++) {
        sd_sum += (v[j] - avg) * (v[j] - avg); //abs(v[j]); //**2;
    }
    double sd = sqrt(sd_sum/(v.size() - 1));
    for (int i = 0; i < v.size(); i++) {
        normalized_vector.push_back((v[i] - avg)/sd);
    }
    return normalized_vector;
}

///Take a simple file and read in, line by line, into a str vector
vector<string> simple_line_read(const string fn) {
    vector<string> v;
    string line;
    ifstream ifs(fn.c_str());
    if (ifs.is_open()) {
        while ( getline (ifs, line) ) {
            v.push_back(line);
        }
    }
    ifs.close();
    return v;
}

//Read in a whole file of text as a long string, return string
string simple_read(const string fn) {
    ifstream t(fn);
    stringstream buffer;
    buffer << t.rdbuf();
    string s = buffer.str();
    return s;
}

//Convert scientific notation string to a double
double scitod(const string s) {
    int e_location = s.find("e");
    if (e_location == string::npos) {return stod(s);}
    else {
        return (stod(s.substr(0, e_location )) * pow(10, stod(s.substr(e_location + 1, s.length() - e_location + 1)) ));
    }
}

//Turn string of space separated scientific numbers into vector of doubles
vector<double> sep_row(const string s) {
    string new_num = "";
    vector<double> v;
    for (int i = 0; i < s.length(); i++) {
        if (s[i] == ',') {
            v.push_back((scitod(new_num)));
            new_num = "";
            
        }
        else {new_num += s[i];}
    }
    v.push_back((scitod(new_num) ));
    return v;
}


///READ in normalized values (note that normalization happens here)
vector< vector<double> > read_trans_matrix(const string fn) {
    vector <vector<double> > trans_matrix;
    vector<string> temp = simple_line_read(fn);
    for (int i = 0; i < temp.size(); i++) {
        trans_matrix.push_back(normalize_vector(sep_row(temp[i])));
    }
    return trans_matrix;
}





double cosine_similarity(vector<double> &A, vector<double> &B)
{
    double dot = 0.0, denom_a = 0.0, denom_b = 0.0 ;
    for(int i = 0; i < A.size(); i++) {
        dot += A[i] * B[i] ;
        denom_a += A[i] * A[i] ;
        denom_b += B[i] * B[i] ;
    }
    return -1 * dot / (sqrt(denom_a) * sqrt(denom_b)) ;
}



///These two functions take a set of ints as input and delete those from vector
vector <vector<double> > delete_multiple_items_from_vector_matrix(vector <vector<double> > &v, set<int> s) {
    vector <vector<double> > new_mat;
    for (int i = 0; i < v.size(); i++) {
        if (s.count(i) == 0) {new_mat.push_back(v[i]);}
    }
    return new_mat;
}

vector <string> delete_multiple_items_from_string_vector(vector <string > &v, set<int> s) {
    vector <string > new_mat;
    for (int i = 0; i < v.size(); i++) {
        if (s.count(i) == 0) {new_mat.push_back(v[i]);}
    }
    return new_mat;
}

vector<double> get_vector_of_size(int s) {
    vector<double> v;
    v.resize(s);
    return v;
}

#endif /* helpers_h */
