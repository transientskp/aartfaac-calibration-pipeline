#include "../stream.h"
#include <gtest/gtest.h>
#include <glog/logging.h>

using namespace ::testing;
using boost::asio::ip::tcp;

DEFINE_int32(subband, -1, "Lofar subband that defines the frequency of incoming data");

class StreamTest : public Test {

protected:
  virtual void SetUp() {
  }

  virtual void TearDown() {
  }
};

TEST_F(StreamTest, DeinterleaveIndex0) {
  Datum s(16*sizeof(uint64_t), 0);
  Datum a(512+s.size()/2, 0);
  Datum b(512+s.size()/2, 0);
  uint64_t *sp = reinterpret_cast<uint64_t*>(s.data());
  for (int i = 0, n = s.size()/8; i < n; i++)
    sp[i] = i;
  Stream::Deinterleave(s, a, b, 0);

  uint64_t *ap = reinterpret_cast<uint64_t*>(a.data()+512);
  uint64_t *bp = reinterpret_cast<uint64_t*>(b.data()+512);
  for (int i = 0, n = (a.size()-512)/8; i < n; i++)
    EXPECT_EQ(ap[i], i*2);
  for (int i = 0, n = (b.size()-512)/8; i < n; i++)
    EXPECT_EQ(bp[i], i*2+1);
}

TEST_F(StreamTest, DeinterleaveFull) {
  Datum s(41616*4*sizeof(uint64_t), 0);
  Datum a(512+s.size()/2, 0);
  Datum b(512+s.size()/2, 0);
  uint64_t *sp = reinterpret_cast<uint64_t*>(s.data());
  for (int i = 0, n = s.size()/8; i < n; i++)
    sp[i] = i;

  Datum buf(166464, 0);
  for (int i = 0, n = s.size()/buf.size(); i < n; i++)
  {
    memcpy(buf.data(), s.data()+i*buf.size(), buf.size());
    Stream::Deinterleave(buf, a, b, i*buf.size()/16);
  }

  uint64_t *ap = reinterpret_cast<uint64_t*>(a.data()+512);
  uint64_t *bp = reinterpret_cast<uint64_t*>(b.data()+512);
  for (int i = 0, n = (a.size()-512)/8; i < n; i++)
    EXPECT_EQ(ap[i], i*2);
  for (int i = 0, n = (b.size()-512)/8; i < n; i++)
    EXPECT_EQ(bp[i], i*2+1);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  FLAGS_gtest_death_test_style = "threadsafe";
  return RUN_ALL_TESTS();
}
