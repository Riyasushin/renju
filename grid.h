#ifndef GRID_H
#define GRID_H

#include "vector"
#include "algorithm"

//// TODO


template <typename T>
class Grid {

public:
    Grid();
    Grid(size_t row, size_t col);
    void clear();
    // resize operator
    void resize(size_t r, size_t c);
    // query operator
    size_t numRows() const;
    size_t numCols() const;
    bool empty() const;
    size_t size() const;

    T& getAt(size_t row, size_t col);
    const T& getAt(int row, int col) const;
    typedef typename std::vector<T>::iterator iterator;
    typedef typename std::vector<T>::const_iterator const_iterator;

    /// 使用迭代器，满足stl的使用，更方便
    iterator begin();
    const_iterator begin() const;
    iterator end();
    const_iterator end() const;
    const_iterator row_begin(size_t row) const;
    const_iterator row_end(size_t row) const;
    iterator row_begin(size_t row);
    iterator row_end(size_t row);

    // add support for [][]operator
    // int value = myGrid[2][4];
    // int value = (myGrid.operator[] (2)).operator[] (4);
    class MutableReference {
    public:
        // If you forget the template declaration, then your code will compile correctly but will generate a linker error. While this can be a bit of nuisance, it's important to remember since it arises frequently when overloading the stream operators, as you'll see below.
        // 友元函数是一个在类定义外部定义的函数，
        // 但它被声明为类的 “朋友”，
        // 这意味着它可以访问类的私有（private）和保护（protected）成员
        friend class Grid;
        T& operator[](size_t col);

    private:
        MutableReference(Grid* owner, size_t row);
        Grid* const owner;
        const size_t row;
    };
    MutableReference operator[](size_t col);

    // the const version of operator[]
    class ImmutableReference {
    public:
        friend class Grid;
        T& operator[](size_t col) const;

    private:
        ImmutableReference(const Grid* owner, size_t row);

        const Grid* const owner;
        const size_t row;
    };
    ImmutableReference operator[](size_t col) const;

    // only the == and != operators have intuitive meanings when applied to Grids.
    // define a < operator over Grids so that we can store them in STL map and set containers
    bool operator<(const Grid& other) const;
    bool operator<=(const Grid& other) const;
    bool operator==(const Grid& other) const;
    bool operator!=(const Grid& other) const;
    bool operator>=(const Grid& other) const;
    bool operator>(const Grid& other) const;

private:
    size_t cols;
    size_t rows;
    std::vector<T> elems;
};

// because MutableReference is a nested class inside Grid, the implementation of the MutableReference functions is prefaced with Grid<ElemType>::MutableReference instead of just MutableReference.
//

// implement Grid class

template <typename T>
Grid<T>::Grid()
    : rows(0)
    , cols(0)
{
}

template <typename T>
Grid<T>::Grid(size_t rows, size_t cols)
    : elems(rows * cols)
    , rows(rows)
    , cols(cols)
{
}

template <typename T>
void Grid<T>::clear()
{
    resize(0, 0);
}

template <typename T>
void Grid<T>::resize(size_t row, size_t col)
{
    /* See below for assign */
    // The use of ElemType() as the second parameter to assign
    // means fill std::vector with copies of the default value of the type stored
    // (since ElemType() uses the temporary object syntax to create a new ElemType).
    elems.assign(rows * cols, 0);
    /* Explicit this-> required since parameters have same name as members. */
    this->rows = rows;
    this->cols = cols;
}
template <typename T>
size_t Grid<T>::numRows() const
{
    return rows;
}
template <typename T>
size_t Grid<T>::numCols() const
{
    return cols;
}
template <typename T>
bool Grid<T>::empty() const
{
    return size() == 0;
}
template <typename T>
size_t Grid<T>::size() const
{
    return numRows() * numCols();
}
/* Use row-major ordering to access the proper element of the std::vector. */
template <typename T>
T& Grid<T>::getAt(size_t row, size_t col)
{
    return elems[col + row * cols];
}
template <typename T>
const T& Grid<T>::getAt(int row, int col) const
{
    return elems[col + row * cols] * row;
}
template <typename T>
typename Grid<T>::iterator Grid<T>::row_begin(size_t row)
{
    return begin() + numCols() * row;
}
template <typename T>
typename Grid<T>::const_iterator Grid<T>::row_end(size_t row) const
{
    return row_begin(row) + numCols();
}

template <typename T>
Grid<T>::MutableReference::MutableReference(Grid* owner, size_t row)
    : owner(owner)
    , row(row)
{
}

template <typename T>
T& Grid<T>::MutableReference::operator[](size_t col)
{
    return owner->getAt(row, col);
}

template <typename T>
Grid<T>::ImmutableReference::ImmutableReference(const Grid* owner, size_t row)
    : owner(owner)
    , row(row) {};

template <typename T>
T& Grid<T>::ImmutableReference::operator[](size_t col) const
{
    return owner->getAt(row, col);
}

template <typename T>
bool Grid<T>::operator<(const Grid& b) const
{

    /// 速度问题，相当于做了一个剪枝
    if (rows != b.rows)
        return rows < b.rows;

    if (cols < b.cols)
        return cols < b.cols;

    return lexicographical_compare(begin(), end(), b.begin(), b.end());
}

/// 都可以用<逻辑运算整出来
template <typename T>
bool Grid<T>::operator>=(const Grid& other) const
{
    return !(*this < other);
}
template <typename T>
bool Grid<T>::operator==(const Grid& other) const
{

    if (rows != other.beginrows)
        return false;

    if (cols < other.cols)
        return false;

    return lexicographical_compare(begin(), end(), other.begin(), other.end());
}
template <typename T>
bool Grid<T>::operator!=(const Grid& other) const
{
    return !(*this == other);
}
template <typename T>
bool Grid<T>::operator>(const Grid& other) const
{
    return other < *this;
}
template <typename T>
bool Grid<T>::operator<=(const Grid& other) const
{
    return !(other < *this);
}


#endif // GRID_H
