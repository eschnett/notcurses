#include "main.h"
#include <cstring>
#include <iostream>

TEST_CASE("Plot") {
  auto nc_ = testing_notcurses();
  if(!nc_){
    return;
  }
  if(!notcurses_canutf8(nc_)){
    CHECK(0 == notcurses_stop(nc_));
    return;
  }
  auto n_ = notcurses_stdplane(nc_);
  REQUIRE(n_);
  REQUIRE(0 == ncplane_cursor_move_yx(n_, 0, 0));

  // setting miny == maxy with non-zero domain limits is invalid
  SUBCASE("DetectRangeBadY"){
    ncplot_options popts{};
    auto p = ncuplot_create(n_, &popts, -1, -1);
    CHECK(nullptr == p);
    p = ncuplot_create(n_, &popts, 1, 1);
    CHECK(nullptr == p);
    p = ncuplot_create(n_, &popts, 0, 0);
    REQUIRE(nullptr != p);
    ncuplot_destroy(p);
  }

  // maxy < miny is invalid
  SUBCASE("RejectMaxyLessMiny"){
    ncplot_options popts{};
    auto p = ncuplot_create(n_, &popts, 2, 1);
    CHECK(nullptr == p);
  }

  SUBCASE("SimplePlot"){
    ncplot_options popts{};
    auto p = ncuplot_create(n_, &popts, 0, 0);
    REQUIRE(p);
    CHECK(n_ == ncuplot_plane(p));
    ncuplot_destroy(p);
  }

  // 5-ary slot space without any window movement
  SUBCASE("AugmentSamples5"){
    ncplot_options popts{};
    popts.rangex = 5;
    struct ncuplot* p = ncuplot_create(n_, &popts, 0, 10);
    uint64_t y;
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(0 == y);
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)0, (uint64_t)1));
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(1 == y);
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)0, (uint64_t)1));
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(2 == y);
    CHECK(-1 == ncuplot_sample(p, 1, &y));
    CHECK(-1 == ncuplot_sample(p, 2, &y));
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)2, (uint64_t)3));
    CHECK(0 == ncuplot_sample(p, 2, &y));
    CHECK(3 == y);
    CHECK(0 == ncuplot_set_sample(p, (uint64_t)2, (uint64_t)3));
    CHECK(0 == ncuplot_sample(p, 2, &y));
    CHECK(3 == y);
    CHECK(-1 == ncuplot_sample(p, 3, &y));
    CHECK(-1 == ncuplot_sample(p, 4, &y));
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)4, (uint64_t)6));
    CHECK(0 == ncuplot_sample(p, 4, &y));
    CHECK(6 == y);
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(2 == y);
    //CHECK(4 == p->slotx);
    ncuplot_destroy(p);
  }

  // 2-ary slot space with window movement
  SUBCASE("AugmentCycle2"){
    ncplot_options popts{};
    popts.rangex = 2;
    struct ncuplot* p = ncuplot_create(n_, &popts, 0, 10);
    uint64_t y;
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)0, (uint64_t)1));
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(1 == y);
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)0, (uint64_t)1));
    CHECK(0 == ncuplot_sample(p, 0, &y));
    CHECK(2 == y);
    CHECK(0 == ncuplot_set_sample(p, (uint64_t)1, (uint64_t)5));
    CHECK(0 == ncuplot_sample(p, 1, &y));
    CHECK(5 == y);
    CHECK(0 == ncuplot_set_sample(p, (uint64_t)2, (uint64_t)9));
    CHECK(0 == ncuplot_sample(p, 1, &y));
    CHECK(5 == y);
    CHECK(-1 == ncuplot_sample(p, 0, &y));
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)3, (uint64_t)4));
    CHECK(-1 == ncuplot_sample(p, 0, &y));
    CHECK(-1 == ncuplot_sample(p, 1, &y));
    //CHECK(3 == p->slotx);
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)5, (uint64_t)1));
    CHECK(-1 == ncuplot_sample(p, 0, &y));
    CHECK(-1 == ncuplot_sample(p, 1, &y));
    //CHECK(5 == p.slotx);
    ncuplot_destroy(p);
  }

  // augment past the window, ensuring everything gets zeroed
  SUBCASE("AugmentLong"){
    ncplot_options popts{};
    popts.rangex = 5;
    struct ncuplot* p = ncuplot_create(n_, &popts, 0, 10);
    uint64_t y;
    CHECK(0 == ncuplot_sample(p, 0, &y));
    for(int x = 1 ; x < 5 ; ++x){
      CHECK(-1 == ncuplot_sample(p, x, &y));
    }
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)4, (uint64_t)4));
    for(int x = 1 ; x < 4 ; ++x){
      CHECK(0 == ncuplot_sample(p, x, &y));
    }
    CHECK(0 == ncuplot_sample(p, 4, &y));
    CHECK(4 == y);
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)10, (uint64_t)5));
    for(int x = 0 ; x < 4 ; ++x){
      CHECK(-1 == ncuplot_sample(p, x, &y));
    }
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)24, (uint64_t)7));
    for(int x = 0 ; x < 5 ; ++x){
      CHECK(-1 == ncuplot_sample(p, x, &y));
    }
    CHECK(0 == ncuplot_add_sample(p, (uint64_t)100, (uint64_t)0));
    for(int x = 0 ; x < 5 ; ++x){
      CHECK(-1 == ncuplot_sample(p, x, &y));
    }
    ncuplot_destroy(p);
  }

  //  FIXME need some high-level rendering tests, one for each geometry

  SUBCASE("SimpleFloatPlot"){
    ncplot_options popts{};
    auto p = ncdplot_create(n_, &popts, 0, 0);
    REQUIRE(p);
    CHECK(n_ == ncdplot_plane(p));
    ncdplot_destroy(p);
  }

  CHECK(0 == notcurses_stop(nc_));
}
