#pragma once

#include <iostream>
#include <iterator>
#include <cmath>
#include <vector>

template <typename T>
class Iterator;

template <typename T>
class Matrix {
private:
    T** _values;
    int _rows, _cols;

    //возможно придётся описать вспомогательный класс строки для перегрузки [][]

public:
    //конструктор
    Matrix(unsigned rows, unsigned cols) {
        //брошено исключение если переданы нулевые размерности
        if (!(rows > 0 && cols > 0))
        {
            throw std::invalid_argument("Matrix dimensions can't be zero");
        }
        _rows = rows;
        _cols = cols;

        _values = (T**) new T * [_rows];

        for (int i = 0; i < _rows; i++)
            _values[i] = (T*) new T[_cols];

        for (int i = 0; i < _rows; i++)
            for (int j = 0; j < _cols; ++j) {
                _values[i][j] = 0;
            }

    }

    //деструктор
    ~Matrix() {
        for (int i = 0; i < _rows; i++)
            delete[] _values[i];
        delete[] _values;
    }

    //конструктор копирования
    Matrix(const Matrix<T>& matrix) {
        _rows = matrix._rows;
        _cols = matrix._cols;

        _values = (T**) new T * [_rows];

        for (int i = 0; i < _rows; i++) {
            _values[i] = (T*) new T[_cols];
        }
        for (int i = 0; i < _rows; i++) {
            for (int j = 0; j < _cols; j++) {
                _values[i][j] = matrix._values[i][j];
            }
        }
    }

    //перегрузка оператора присваивания
    Matrix<T>& operator=(const Matrix<T>& matrix) {
        if (this == &matrix) //проверили не тот же ли это элемент
            return *this;

        if (_rows != matrix._rows || _cols != matrix._cols) { //если размеры не равны, то очищаем память для *this и выделяем заново
            for (int i = 0; i < _rows; i++)
                delete[] _values[i];
            delete[] _values;

            _rows = matrix._rows;
            _cols = matrix._cols;

            _values = (T**) new T * [matrix._rows];
            for (int i = 0; i < matrix._rows; i++) {
                _values[i] = (T*) new T[matrix._cols];
            }
        }

        for (int i = 0; i < _rows; i++) {
            for (int j = 0; j < _cols; j++) {
                _values[i][j] = matrix._values[i][j];
            }
        }
        return *this;
    }

    template <typename T1> //чтобы не было перекрытия шаблонного параметра
    friend std::ostream& operator<<(std::ostream& os, const Matrix<T1>& matrix) {
        for (int i = 0; i < matrix._rows; i++) {
            os << '[';
            for (int j = 0; j < matrix._cols; j++) {
                os << matrix._values[i][j];
                if (j + 1 != matrix._cols) {
                    os << '\t';
                }
            }
            os << "]\n";
        }
        return os;
    }

    T& getValue(const int i, const int j) {
        return this->_values[i][j];
    }

    const T& getValue(const int i, const int j) const {
        return this->_values[i][j];
    }

    int getRows() { return _rows; };
    int getCols() { return _cols; };

    Iterator<T> begin();

    Iterator<T> end();
};

template <typename T>
class Iterator {
private:
    T* _begin, * _end, * _ptr;
    int _i, _j, _pos; //pos - номер в улиточном порядке на отрезке [0; rows*cols-1]
    T** _values;
    int _rows, _cols;

public:
    using iterator_category = std::random_access_iterator_tag; //объявили категорию итератора (это тип данных)
    using difference_type = std::ptrdiff_t; //разница между итераторами (также тип данных)
    using value_type = T; //указываем тип данных
    using pointer = value_type*;//тип данных указателя
    using reference = value_type&;
    using const_reference = const value_type&;

    void pos_to_ij() {
        if (_pos < _rows - 1) {
            _i = _pos;
            _j = _cols - 1;
        }
        else {
            int tmp = _pos - _rows + 1;
            int n = (int)((_rows + _cols - 1 - sqrt((_rows + _cols - 1) * (_rows + _cols - 1) - 4 * tmp)) / 2. + 1.);
            tmp -= (n - 1) * (_rows + _cols - n);
            if (n % 2 == 0) {
                if (tmp >= _cols - n) {
                    _i = n / 2 - 1 + tmp - _cols + n;
                    _j = _cols - 1 - n / 2;
                }
                else {
                    _i = n / 2 - 1;
                    _j = n / 2 - 1 + tmp;
                }
            }
            else {
                if (tmp >= _cols - n) {
                    _i = _rows - (n + 1) / 2 - (tmp - _cols + n);
                    _j = (n - 1) / 2;
                }
                else {
                    _i = _rows - (n + 1) / 2;
                    _j = _cols - (n + 1) / 2 - tmp;
                }
            }
        }
    }


    Iterator(pointer begin, pointer end, pointer ptr, int pos, int rows, int cols, value_type** values) :
        _begin(begin), _end(end), _ptr(ptr), _pos(pos), _rows(rows), _cols(cols), _values(values) {
        pos_to_ij();
    }

    bool operator==(const Iterator& it) const { return  _pos == it._pos; }
    bool operator!=(const Iterator& it) const { return _pos != it._pos; }
    bool operator<(const Iterator& it) const { return _pos - it._pos < 0; }
    bool operator<=(const Iterator& it) const { return _pos - it._pos <= 0; }
    bool operator>(const Iterator& it) const { return _pos - it._pos > 0; }
    bool operator>=(const Iterator& it) const { return _pos - it._pos >= 0; }

    reference operator*() {
        if (_pos == _rows * _cols)
            throw std::out_of_range("Dereference of an end pointer");
        return *_ptr;
    } // проверка чтобы не разыменовать end с пробросом искл

    const_reference operator*() const {
        if (_pos == _rows * _cols)
            throw std::out_of_range("Dereference of an end pointer");
        return *_ptr;
    }

    Iterator& operator++() //++it
    {
        if (_pos == _rows * _cols - 1) {
            *this = Iterator<T>(this->_begin,
                static_cast<T*>(nullptr),
                static_cast<T*>(nullptr),
                _rows * _cols, this->_rows,
                this->_cols, this->_values);
            return *this;
        }

        _pos++;
        pos_to_ij();
        _ptr = &(_values[_i][_j]);

        return *this;
    }

    Iterator operator++(int) //it++
    {
        Iterator prev = *this;

        *this = ++(*this);

        return prev;
    }

    Iterator& operator--() //--it
    {
        if (_pos == _rows * _cols) {
            *this = Iterator(this->_begin, this->_end,
                this->_ptr, this->_pos - 1,
                this->_rows, this->_cols,
                this->_values);
            return *this;
        }

        if (_ptr != _begin) {
            _pos--;
            pos_to_ij();
            _ptr = &(_values[_i][_j]);
        }

        return *this;
    }

    Iterator operator--(int) //it--
    {
        Iterator prev = *this;

        *this = --(*this);

        return prev;
    }

    Iterator operator+(difference_type n) const {
        Iterator shiftedIt = *this;

        for (difference_type i = 0; i < n; i++) {
            ++shiftedIt;
        }
        return shiftedIt;
    }

    Iterator operator+=(difference_type n) {

        for (difference_type i = 0; i < n; i++) {
            ++(*this);
        }
        return *this;
    }

    Iterator operator-(difference_type n) const {
        Iterator shiftedIt = *this;

        for (difference_type i = 0; i < n; i++) {
            --shiftedIt;
        }
        return shiftedIt;
    }

    Iterator operator-=(difference_type n) {

        for (difference_type i = 0; i < n; i++) {
            --(*this);
        }
        return *this;
    }

    difference_type operator-(const Iterator& it) const {
        return _pos - it._pos;
    }

    //дружественная перегрузка для того чтобы делать не it + 5, а ещё и 5 + it (коммутативность)
    /*template <typename T1>
    friend typename Matrix<T1>::Iterator operator+(difference_type n, const typename Matrix<T1>::Iterator& it) const {
        typename Matrix<T1>::Iterator shiftedIt = *this;

        for (difference_type i = 0; i < n; i++) {
            if (shiftedIt._ptr == shiftedIt._end)
                return shiftedIt;

            shiftedIt._pos++;
            shiftedIt.pos_to_ij();
            shiftedIt._ptr = &(_values[_i][_j]);
        }
        return shiftedIt;
    }
    */
};

//итератор окончания смотрит на место ЗА последней ячейкой контейнера
template <typename T>
Iterator<T> Matrix<T>::begin() {
    return Iterator<T>(&(_values[0][_cols - 1]), static_cast<T*>(nullptr), &(_values[0][_cols - 1]), 0, _rows, _cols, _values);
}
//добавляю размерности костылём в begin и end

template <typename T>
Iterator<T> Matrix<T>::end() {
    T* end;
    if (_rows < _cols) {
        if (_rows % 2 == 0) {
            end = &(_values[_rows / 2 - 1][_cols - 1 - _rows / 2]);
        }
        else {
            end = &(_values[_rows - _rows / 2 - 1][_rows / 2]);
        }
    }
    else {
        if (_cols % 2 == 0) {
            end = &(_values[_cols / 2 - 1][_cols / 2 - 1]);
        }
        else {
            end = &(_values[_rows - _cols / 2 - 1][_cols / 2]);
        }
    }
    return Iterator<T>(&(_values[0][_cols - 1]), static_cast<T*>(nullptr), end, _rows * _cols, _rows, _cols, _values);
}
