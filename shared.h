// shared.h

#ifndef SHARED_H
#define SHARED_H

// This enum defined the API for our program array (for tail calls)
// It's the single source of truth for al tail call indexes
enum tail_call_targets {
  TAIL_CALL_PROG_1 = 0,
  TAIL_CALL_PROG_2 = 1,
  MAX_TAIL_CALLS, // Holds the number of tail calls
};

#endif // SHARED_H
