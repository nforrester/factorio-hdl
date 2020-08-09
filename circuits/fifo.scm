(load "circuits/util.scm")

; A FIFO. End to end latency is 4 cycles. Upon reset it is empty. When empty it emits default-value.
; Popping when empty is not an error.
; Overflow because you pushed too much and popped too little is an error.
(define defpart-fifo-n
  (lambda (size)
    `(defpart ,(strings->symbol "fifo-" (number->string size))
       ((in red data-in)
        (out red data-out)
        (in red push)
        (in red pop)
        (in red default-value)
        (in red reset)
        (out yellow errors)
        (signal data-signal)
        (signal control-signal)
        (signal reset-signal)
        (signal error-signal))

       ; Write and read pointer addresses are modular.
       ; 0 is the same as size, 1 is the same as size + 1, etc.
       ; This relaxes the requirement to wrap them around at the exactly correct moment,
       ; or exactly once per revolution through the buffer.
       ; It is still important to wrap them because otherwise they'd grow without bound
       ; and eventually overflow. Yes, that would require more than two years of continuous
       ; operation, but it's the principle of the thing.
       (signal address-signal (notsigs data-signal control-signal reset-signal error-signal))
       (green read-ptr)
       (green write-ptr)

       ; We track the fill level as a negative number so that we can exploit the everything
       ; wildcard when popping to gate the pop on minus-fill-level being non-zero.
       (green minus-fill-level)

       (red minus-size-red)
       (green minus-size-green)
       (constant (minus-size-red minus-size-green) ((address-signal ,(* -1 size))))

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
       ; modulo the size of the fifo. It also guarantees that read-ptr is sensibly
       ; ordered with respect to write-ptr at all times by wrapping them in lock step.
       (signal need-to-wrap-sig sig:signal-w)
       (green need-to-wrap)
       (decider write-ptr need-to-wrap address-signal > ,(* 3 size) need-to-wrap-sig one)
       (decider (need-to-wrap minus-size-red) read-ptr  need-to-wrap-sig == 1 address-signal input-count)
       (decider (need-to-wrap minus-size-red) write-ptr need-to-wrap-sig == 1 address-signal input-count)

       ; delay 1
       ; 0 <= write-ptr-mod-maybe < size, unless push == 0
       (green read-ptr-mod)
       (green write-ptr-mod-maybe)
       (arithmetic read-ptr  read-ptr-mod  address-signal % ,size address-signal)
       (arithmetic write-ptr write-ptr-mod-maybe address-signal % ,size address-signal)
       (decider (push minus-size-green) write-ptr-mod-maybe control-signal == 0 address-signal input-count)

       ; delay 1
       (red data-in-delayed1)
       (buffer data-in data-in-delayed1)

       ; delay 1
       ; Detect empty
       (green empty)
       (decider minus-fill-level empty address-signal == 0 control-signal one)

       ; delay 2
       (green empty-delayed1)
       (buffer empty empty-delayed1)

       ; delay 2
       ; The buffer
       (red data-out-raw)
       (,(strings->symbol "fast-memory-start-0-width-" (number->string size))
         data-in-delayed1 data-out-raw
         write-ptr-mod-maybe read-ptr-mod
         data-signal address-signal address-signal)

       ; Delay default-value
       (red default-value-delayed1)
       (red default-value-delayed2)
       (red default-value-delayed3)
       (buffer default-value default-value-delayed1)
       (buffer default-value-delayed1 default-value-delayed2)
       (buffer default-value-delayed2 default-value-delayed3)

       ; delay 3
       (decider (data-out-raw empty-delayed1) data-out control-signal == 0 data-signal input-count)
       (decider (default-value-delayed3 empty-delayed1) data-out control-signal == 1 data-signal input-count)

       ; Detect buffer overflow
       (red errors-early2)
       (red errors-early1)
       ; delay 1
       (decider minus-fill-level errors-early2 address-signal == ,(- (* -1 size) 1) error-signal one)
       ; delay 2
       (buffer errors-early2 errors-early1)
       ; delay 3
       (buffer errors-early1 errors))))
