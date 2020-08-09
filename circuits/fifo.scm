(load "circuits/util.scm")

; A FIFO. End to end latency is 4 cycles. Upon reset it is empty. When empty it emits data-default.
; Popping when empty is not an error.
; Overflow because you pushed too much and popped too little is an error.
(define defpart-fifo-length-n-width-m
  (lambda (length width)
    `(begin
       (defpart-fast-memory-start-n-width-m 0 ,length)
       (defpart ,(strings->symbol "fifo-length-" (number->string length) "-width-" (number->string width))
         (,@(apply append
             (for (range width)
               (lambda (index)
                 `((in  red ,(sym-for-idx 'data-in index))
                   (out red ,(sym-for-idx 'data-out index))
                   (in  red ,(sym-for-idx 'data-default index))
                   (signal ,(sym-for-idx 'data-signal index))))))
          (in red push)
          (in red pop)
          (in red reset)
          (out yellow errors)
          (signal control-signal)
          (signal reset-signal)
          (signal error-signal))

         ; Write and read pointer addresses are modular.
         ; 0 is the same as length, 1 is the same as length + 1, etc.
         ; This relaxes the requirement to wrap them around at the exactly correct moment,
         ; or exactly once per revolution through the buffer.
         ; It is still important to wrap them because otherwise they'd grow without bound
         ; and eventually overflow. Yes, that would require more than two years of continuous
         ; operation, but it's the principle of the thing.
         (signal
           address-signal
           (notsigs
             control-signal
             reset-signal
             error-signal
             ,@(apply append
                (for (range width)
                  (lambda (index)
                    (list (sym-for-idx 'data-signal index)))))))
         (green read-ptr)
         (green write-ptr)

         ; We track the fill level as a negative number so that we can exploit the everything
         ; wildcard when popping to gate the pop on minus-fill-level being non-zero.
         (green minus-fill-level)

         (red minus-length-red)
         (green minus-length-green)
         (constant (minus-length-red minus-length-green) ((address-signal ,(* -1 length))))

         ; Preserve pointer values and fill level, but allow reset
         (decider (reset read-ptr)         read-ptr         reset-signal == 0 address-signal input-count)
         (decider (reset write-ptr)        write-ptr        reset-signal == 0 address-signal input-count)
         (decider (reset minus-fill-level) minus-fill-level reset-signal == 0 address-signal input-count)

         ; Increment pointers on push and pop
         ; Don't increment read-ptr if minus-fill-level == 0.
         (decider (pop minus-fill-level) read-ptr control-signal == 1 everything one)
         (decider push write-ptr control-signal == 1 address-signal one)

         ; Track fill level
         (green minus1)
         (constant minus1 ((address-signal -1)))
         (decider (pop minus-fill-level) minus-fill-level control-signal == 1 everything one)
         (arithmetic pop minus-fill-level control-signal * -1 control-signal)
         (decider (push minus1) minus-fill-level control-signal == 1 address-signal input-count)

         ; Wrap around to prevent overflow. This doesn't guarantee anything about the
         ; timing of the wrapping, only that it will happen well before integer
         ; overflow and that at any given moment the pointer values will be correct
         ; modulo the length of the fifo. It also guarantees that read-ptr is sensibly
         ; ordered with respect to write-ptr at all times by wrapping them in lock step.
         (signal need-to-wrap-sig sig:signal-w)
         (green need-to-wrap)
         (decider write-ptr need-to-wrap address-signal > ,(* 3 length) need-to-wrap-sig one)
         (decider (need-to-wrap minus-length-red) read-ptr  need-to-wrap-sig == 1 address-signal input-count)
         (decider (need-to-wrap minus-length-red) write-ptr need-to-wrap-sig == 1 address-signal input-count)

         ; delay 1
         ; 0 <= write-ptr-mod-maybe < length, unless push == 0
         (green read-ptr-mod)
         (green write-ptr-mod-maybe)
         (arithmetic read-ptr  read-ptr-mod  address-signal % ,length address-signal)
         (arithmetic write-ptr write-ptr-mod-maybe address-signal % ,length address-signal)
         (decider (push minus-length-green) write-ptr-mod-maybe control-signal == 0 address-signal input-count)

         ; delay 1
         ; Detect empty
         (green empty)
         (decider minus-fill-level empty address-signal == 0 control-signal one)

         ; delay 2
         (green empty-delayed1)
         (buffer empty empty-delayed1)

         ,@(apply append
            (for (range width)
              (lambda (index)
                `(; delay 1
                  (red ,(sym-for-idx 'data-in-delayed1 index))
                  (buffer ,(sym-for-idx 'data-in index) ,(sym-for-idx 'data-in-delayed1 index))

                  ; delay 2
                  ; The buffer
                  (red ,(sym-for-idx 'data-out-raw index))
                  (,(strings->symbol "fast-memory-start-0-width-" (number->string length))
                    ,(sym-for-idx 'data-in-delayed1 index) ,(sym-for-idx 'data-out-raw index)
                    write-ptr-mod-maybe read-ptr-mod
                    ,(sym-for-idx 'data-signal index) address-signal address-signal)

                  ; Delay data-default
                  (red ,(sym-for-idx 'data-default-delayed1 index))
                  (red ,(sym-for-idx 'data-default-delayed2 index))
                  (red ,(sym-for-idx 'data-default-delayed3 index))
                  (buffer ,(sym-for-idx 'data-default index) ,(sym-for-idx 'data-default-delayed1 index))
                  (buffer ,(sym-for-idx 'data-default-delayed1 index) ,(sym-for-idx 'data-default-delayed2 index))
                  (buffer ,(sym-for-idx 'data-default-delayed2 index) ,(sym-for-idx 'data-default-delayed3 index))

                  ; delay 3
                  (decider
                    (,(sym-for-idx 'data-out-raw index) empty-delayed1)
                    ,(sym-for-idx 'data-out index)
                    control-signal == 0 ,(sym-for-idx 'data-signal index) input-count)
                  (decider
                    (,(sym-for-idx 'data-default-delayed3 index) empty-delayed1)
                    ,(sym-for-idx 'data-out index)
                    control-signal == 1 ,(sym-for-idx 'data-signal index) input-count)))))

         ; Detect buffer overflow
         (red errors-early2)
         (red errors-early1)
         ; delay 1
         (decider minus-fill-level errors-early2 address-signal == ,(- (* -1 length) 1) error-signal one)
         ; delay 2
         (buffer errors-early2 errors-early1)
         ; delay 3
         (buffer errors-early1 errors)))))
