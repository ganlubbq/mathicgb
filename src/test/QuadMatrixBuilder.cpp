#include "mathicgb/stdinc.h"

#include "mathicgb/Poly.hpp"
#include "mathicgb/PolyRing.hpp"
#include "mathicgb/QuadMatrixBuilder.hpp"
#include "mathicgb/io-util.hpp"
#include <gtest/gtest.h>

namespace {
  void createColumns(const char* left, const char* right, QuadMatrixBuilder& b)
  {
    const PolyRing& ring = b.ring();
    {
      Poly p(&b.ring());
      std::istringstream in(left);
      p.parse(in);
      size_t colCount = 0;
      for (Poly::iterator it = p.begin(); it != p.end(); ++it) {
        QuadMatrixBuilder::ColIndex col = b.createColumnLeft(it.getMonomial());
        ASSERT_EQ(colCount, col);
        ++colCount;
        // not equal as pointers
        ASSERT_TRUE(it.getMonomial().unsafeGetRepresentation() !=
                    b.monomialOfLeftCol(col).unsafeGetRepresentation());
        ASSERT_TRUE // equal as values
          (b.ring().monomialEQ(it.getMonomial(), b.monomialOfLeftCol(col)));
      }
      ASSERT_EQ(colCount, b.leftColCount());
    }
    {
      Poly p(&b.ring());
      std::istringstream in(right);
      p.parse(in);
      size_t colCount = 0;
      for (Poly::iterator it = p.begin(); it != p.end(); ++it) {
        QuadMatrixBuilder::ColIndex col = b.createColumnRight(it.getMonomial());
        ASSERT_EQ(colCount, col);
        ++colCount;
        // not equal as pointers
        ASSERT_TRUE(it.getMonomial().unsafeGetRepresentation() !=
                    b.monomialOfRightCol(col).unsafeGetRepresentation());
        ASSERT_TRUE // equal as values
          (b.ring().monomialEQ(it.getMonomial(), b.monomialOfRightCol(col)));
      }
      ASSERT_EQ(colCount, b.rightColCount());
    }
  }
}

TEST(QuadMatrixBuilder, Empty) {
  PolyRing ring(2, 0, 0);
  QuadMatrixBuilder b(ring); // test a builder with no rows and no columns
  const char* matrixStr = 
    "Left columns:\n"
    "Right columns:\n"
    "matrix with no rows | matrix with no rows\n"
    "                    |                    \n"
    "matrix with no rows | matrix with no rows\n";
  ASSERT_EQ(matrixStr, b.toString());
}

TEST(QuadMatrixBuilder, Construction) {
  std::auto_ptr<PolyRing> ring(ringFromString("32003 6 1\n1 1 1 1 1 1"));
  QuadMatrixBuilder b(*ring);
  createColumns("a<1>+<0>", "b<0>+c<0>+bc<0>", b);

  // top row: nothing, nothing
  b.rowDoneTopLeftAndRight();

  // top row: 0#1 1#2, 2#3
  b.appendEntryTopLeft(0, 1);
  b.appendEntryTopLeft(1, 2);
  b.appendEntryTopRight(2, 3);
  b.rowDoneTopLeftAndRight();

  // bottom row: 1#4, nothing
  b.appendEntryBottomLeft(1,4);
  b.rowDoneBottomLeftAndRight();

  // bottom row: nothing, 0#5
  b.appendEntryBottomRight(0,5);
  b.rowDoneBottomLeftAndRight();

  // bottom row: nothing, nothing
  b.rowDoneBottomLeftAndRight();

  const char* matrixStr =
    "Left columns: a 1\n"
    "Right columns: b c bc\n"
    "0:         | 0:    \n"
    "1: 0#1 1#2 | 1: 2#3\n"
    "           |       \n"
    "0: 1#4     | 0:    \n"
    "1:         | 1: 0#5\n"
    "2:         | 2:    \n";
  ASSERT_EQ(matrixStr, b.toString());
}

TEST(QuadMatrixBuilder, ColumnQuery) {
  std::auto_ptr<PolyRing> ring(ringFromString("32003 6 1\n1 1 1 1 1 1"));
  QuadMatrixBuilder b(*ring);
  createColumns("a<1>+<0>", "b<0>+c<0>+bc<0>", b);

  Poly p(&b.ring());
  // coefficient 1X=left, 2X=right, 30=not there, % 10 = column index
  std::istringstream in
    ("10a<1>+11<0>+20b<0>+21c<0>+22bc<0>+30ab<0>+30e<0>+10a<1>");
  p.parse(in);
  for (Poly::iterator it = p.begin(); it != p.end(); ++it) {
    QuadMatrixBuilder::LeftRightColIndex col = b.findColumn(it.getMonomial());
    if (it.getCoefficient() / 10 == 3)
      ASSERT_FALSE(col.valid());
    else {
      ASSERT_TRUE(col.valid());
      ASSERT_EQ(it.getCoefficient() % 10, col.index());
      if (it.getCoefficient() / 10 == 2)
        ASSERT_TRUE(col.right());
      else
        ASSERT_TRUE(col.left());
    }
  }
}
