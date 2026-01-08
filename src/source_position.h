#pragma once

struct source_position {
    int line, pos;
};

struct source_range {
    struct source_position start, end;
};