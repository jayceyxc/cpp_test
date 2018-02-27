#ifndef __BMCC_LABEL_H__
#define __BMCC_LABEL_H__

#include <iostream>
#include <string>
#include <cstdlib>

#define LEN_PER_LEVEL 2
#define MAX_LABEL_LEVEL 4
#define INVALID_LABEL_LEVEL -1

using namespace std;

class DecimalStringLabel
{
public:
    DecimalStringLabel(string id, int max_level, int len_per_level):_label_id(id), _max_level(max_level), _len_per_level(len_per_level) {
        level = INVALID_LABEL_LEVEL;
        if (IsNumeric(id)) {
            level = CalcLevel(id);
			compare_len = CalcCompareLen(id);
            CalcFirstLevelId(id);
        }
    }

    string getLabelId() const {
        return _label_id;
    }

	size_t GetCompareLen() const {
		return compare_len;
	}

    int getLabelLevel() const {
        return level;
    }

    string getFirstLevelId() const {
        return _first_level_id;
    }

    bool operator==(const DecimalStringLabel &right) const {
        int result = _label_id.compare(0, compare_len, right.getLabelId());
        //cout << "call operator ==, " << this->getLabelId() << " compare " << right.getLabelId() << ", compare len: " << this->GetCompareLen() << ", result is " << result << endl;
        //if (_label_id.compare(0, this->GetCompareLen(), right.getLabelId()) == 0) {
        if (result == 0) {
            return true;
        }
        return false;
    }

    bool operator<(const DecimalStringLabel &right) const {
        int result = _label_id.compare(0, compare_len, right.getLabelId(), 0, compare_len);
        //cout << "call operator <, " << this->getLabelId() << " compare " << right.getLabelId() << ", compare len: " << this->GetCompareLen() << ", result is " << result << endl;
        //if (_label_id.compare(0, this->GetCompareLen(), right.getLabelId()) < 0) {
        if (result < 0) {
            //cout << "return true" << endl;
            return true;
        }
        //cout << "return false" << endl;
        return false;
    }
private:
    bool IsNumeric(string &value) const {
        for (string::size_type i = 0; i < value.length(); i++)
        {
            if (value[i] < '0' || value[i] > '9')
            {
                return false;
            }
        }
        return true;
    }

    size_t CalcLevel(string &value) const {
		size_t zero_number = 0;
		for (size_t index = value.length(); index > 0; index--) {
			if (value[index - 1] != '0') {
				break;
			}
			zero_number++;
		}
		if (zero_number == value.length())
		{
			cout << "invalid label id: " << value << endl;
			return INVALID_LABEL_LEVEL;
		}

        return zero_number / _len_per_level;
    }

	size_t CalcCompareLen(string &id) const {
        size_t label_len = id.length();
        return label_len - (_max_level - level) * _len_per_level;
    }

    void CalcFirstLevelId(const string &id) {
        _first_level_id = id;
        size_t erase_len = (_max_level - 1) * _len_per_level;
        _first_level_id.erase(_first_level_id.end() - erase_len, _first_level_id.end());
    }

    size_t compare_len;
    int level;
    string _label_id;
    string _first_level_id;
	int _max_level;
	int _len_per_level;
};

#endif
