(load "circuits/util.scm")

(define defpart-dual-mux-all-signals-n-circuits-offset-by-m
  (lambda (num-circuits address-offset)
    (assert (eqv? 0 (remainder address-offset (len all-signals)))
            (string-append "Memoary address offset must be aligned to "
                           (number->string (len all-signals)) " word intervals."))
    (let ((circuit-offset (quotient address-offset (len all-signals))))
      `(begin
         (defpart-mux-n-circuits-offset-by-m ,num-circuits ,circuit-offset)
         (defpart ,(strings->symbol "dual-mux-all-signals-" (number->string num-circuits)
                                    "-circuits-offset-by-" (number->string address-offset))
           ((in yellow address)
            ,@(for (range num-circuits)
                (lambda (index)
                  `(in yellow ,(sym-for-idx 'input index))))
            (out yellow output)
            (signal address-signal)
            (signal output-signal))
  
           ; delay 1
           (green which-circuit)
           (arithmetic address which-circuit address-signal / ,(len all-signals) address-signal)
  
           ; delay 1
           (green which-signal)
           (arithmetic address which-signal address-signal % ,(len all-signals) address-signal)
  
           ; delay 2
           (green which-signal-delayed1)
           (buffer which-signal which-signal-delayed1)
  
           ; delay 3
           (green which-signal-delayed2)
           (buffer which-signal-delayed1 which-signal-delayed2)
  
           ; delay 1
           ,@(apply append
              (for (range num-circuits)
                (lambda (index)
                  `((green ,(sym-for-idx 'input-delayed index))
                    (buffer ,(sym-for-idx 'input index) ,(sym-for-idx 'input-delayed index))))))
  
           ; delay 3
           (green muxed-circuit)
           (,(strings->symbol "mux-" (number->string num-circuits)
                              "-circuits-offset-by-" (number->string circuit-offset))
              ,@(for (range num-circuits)
                  (lambda (index)
                    (sym-for-idx 'input-delayed index)))
              muxed-circuit
              which-circuit
              address-signal)
  
           ; delay 6
           (mux-all-signals muxed-circuit output which-signal-delayed2 address-signal output-signal))))))

(define defpart-dual-demux-all-signals-n-circuits-offset-by-m
  (lambda (num-circuits address-offset)
    (assert (eqv? 0 (remainder address-offset (len all-signals)))
            (string-append "Memoary address offset must be aligned to "
                           (number->string (len all-signals)) " word intervals."))
    (let ((circuit-offset (quotient address-offset (len all-signals))))
      `(begin
         (defpart-demux-n-circuits-offset-by-m ,num-circuits ,circuit-offset)
         (defpart ,(strings->symbol "dual-demux-all-signals-" (number->string num-circuits)
                                    "-circuits-offset-by-" (number->string address-offset))
           ((in yellow address)
            (in yellow input)
            ,@(for (range num-circuits)
                (lambda (index)
                  `(out yellow ,(sym-for-idx 'output index))))
            (signal address-signal)
            (signal input-signal))

           ; delay 1
           (green which-signal)
           (arithmetic address which-signal address-signal % ,(len all-signals) address-signal)

           ; delay 1
           (green which-circuit)
           (arithmetic address which-circuit address-signal / ,(len all-signals) address-signal)

           ; delay 2
           (green which-circuit-delayed1)
           (buffer which-circuit which-circuit-delayed1)

           ; delay 3
           (green which-circuit-delayed2)
           (buffer which-circuit-delayed1 which-circuit-delayed2)

           ; delay 1
           (green input-delayed)
           (buffer input input-delayed)

           ; delay 3
           (green input-on-sig)
           (demux-all-signals input-delayed input-on-sig which-signal address-signal input-signal)

           ; delay 5
           (,(strings->symbol "demux-" (number->string num-circuits)
                              "-circuits-offset-by-" (number->string circuit-offset))
              input-on-sig
              ,@(for (range num-circuits)
                  (lambda (index)
                    (sym-for-idx 'output index)))
              which-circuit-delayed2
              address-signal))))))

; Define a memory with a specified number of signed 32 bit int registers.
; To write the value of a register:
;   - Tick 0
;     - Set address to the address of the register you want to set (0 indexed).
;     - Set data-in to the value you wish to set.
;     - Set write to 1.
;     - Hold inputs stable for 5 ticks.
;   - Tick 5
;     - Values may now be changed.
;     - You may not request to read the register until 6 ticks after this point.
;   - Tick 11
;     - You may now submit a read request for this register.
; To read the value of a register:
;   - Tick 0
;     - Set address to the address of the register you want to read (0 indexed).
;     - Set write to 0.
;     - There is no special stability time requirement (1 tick required stability time).
;     - The desired value will appear on data-out 6 ticks later.
;   - Tick 6
;     - The value of the register appears on data-out.
; Read and write transactions may be submitted with no intervening delay
; (their processing is pipelined).
; You may read a register and then immediately write it.
; If you write a register you must wait 5 ticks before trying to read it again.
(define defpart-dense-memory-start-n-width-m
  (lambda (start-address width)
    (assert (eqv? 0 (remainder start-address (len all-signals)))
            (string-append "RAM segment start must be aligned to "
                           (number->string (len all-signals)) " word intervals."))
    (assert (eqv? 0 (remainder width (len all-signals)))
            (string-append "RAM segment width must be a multiple of "
                           (number->string (len all-signals)) " words."))
    (let* ((num-cells (quotient width (len all-signals)))
           (dual-mux-all-signals-n-circuits
             (strings->symbol "dual-mux-all-signals-" (number->string num-cells)
                              "-circuits-offset-by-" (number->string start-address)))
           (dual-demux-all-signals-n-circuits
             (strings->symbol "dual-demux-all-signals-" (number->string num-cells)
                              "-circuits-offset-by-" (number->string start-address))))
      `(begin
         (defpart-dual-mux-all-signals-n-circuits-offset-by-m ,num-cells ,start-address)
         (defpart-dual-demux-all-signals-n-circuits-offset-by-m ,num-cells ,start-address)
         (defpart ,(strings->symbol "dense-memory-start-" (number->string start-address)
                                    "-width-" (number->string width))
           ((in yellow address)
            (in yellow write)
            (in yellow data-in)
            (out yellow data-out)
            (signal address-signal)
            (signal write-signal)
            (signal data-in-signal)
            (signal data-out-signal))

           ; Demux data-in to the registers.
           ,@(for (range num-cells)
               (lambda (index)
                 `(green ,(sym-for-idx 'data-in-cell index))))
           (,dual-demux-all-signals-n-circuits
              address
              data-in
              ,@(for (range num-cells)
                  (lambda (index)
                    (sym-for-idx 'data-in-cell index)))
              address-signal
              data-in-signal)

           ; Demux write to the registers.
           ,@(for (range num-cells)
               (lambda (index)
                 `(green ,(sym-for-idx 'write-cell index))))
           (,dual-demux-all-signals-n-circuits
              address
              write
              ,@(for (range num-cells)
                  (lambda (index)
                    (sym-for-idx 'write-cell index)))
              address-signal
              write-signal)

           ; Helper for latch*
           (red all-ones)
           (constant-all-ones all-ones)

           ; The registers.
           ,@(apply append
               (for (range num-cells)
                 (lambda (index)
                   `((green ,(sym-for-idx 'data-out-cell index))
                     (dense-latch*
                      ,(sym-for-idx 'data-in-cell index)
                      ,(sym-for-idx 'data-out-cell index)
                      ,(sym-for-idx 'write-cell index)
                      all-ones)))))

           ; Mux data-out from the registers.
           (,dual-mux-all-signals-n-circuits
              address
              ,@(for (range num-cells)
                  (lambda (index)
                    (sym-for-idx 'data-out-cell index)))
              data-out
              address-signal
              data-out-signal))))))

(define defpart-dense-rom
  (lambda (part-name start-address words)
    (letrec ((align (lambda ()
                      (if (not (eqv? 0 (remainder start-address (len all-signals))))
                        (begin
                          (set! (start-address) (list (- start-address 1)))
                          (set! (words) (list (cons 0 words)))
                          (align))))))
      (align))
    (assert (eqv? 0 (remainder start-address (len all-signals)))
            (string-append "ROM segment start must be aligned to "
                           (number->string (len all-signals)) " word intervals."))
    (let* ((num-words (len words))
           (num-signals (len all-signals))
           (num-cells (+ (quotient num-words num-signals)
                         (if (eqv? 0 (remainder num-words num-signals)) 0 1)))
           (dual-mux-all-signals-n-circuits-offset-by-m
             (strings->symbol "dual-mux-all-signals-" (number->string num-cells)
                              "-circuits-offset-by-" (number->string start-address))))
      `(begin
         (defpart-dual-mux-all-signals-n-circuits-offset-by-m ,num-cells ,start-address)
         (defpart ,part-name
           ((in yellow address)
            (out yellow data-out)
            (signal address-signal)
            (signal data-out-signal))

           ; The constant combinators storing data.
           ,@(apply append
               (for (zip (chunks-of num-signals words) (range num-cells))
                 (lambda+ (chunk-of-words chunk-number)
                   (let ((data-wire (sym-for-idx 'data-out-cell chunk-number)))
                     `((green ,data-wire)
                       ,@(for (chunks-of max-signals-per-constant-combinator
                                         (zip chunk-of-words all-signals))
                           (lambda (chunk-of-words-and-signals)
                             `(constant
                                ,data-wire
                                ,(for chunk-of-words-and-signals
                                   (lambda+ (word signal)
                                     `(,signal ,(unsigned-literal-if-pos word))))))))))))

           ; Mux data-out from the constant combinators.
           (,dual-mux-all-signals-n-circuits-offset-by-m
              address
              ,@(for (range num-cells)
                  (lambda (index)
                    (sym-for-idx 'data-out-cell index)))
              data-out
              address-signal
              data-out-signal))))))

; Writes and reads can happen simultaneously.
; You can write and read one register per cycle.
; To choose not to write anything this cycle, give a write-address that is out of range.
(define defpart-fast-memory-start-n-width-m
  (lambda (start-address width)
    `(defpart ,(strings->symbol "fast-memory-start-" (number->string start-address)
                                "-width-" (number->string width))
       ((in green data-in)
        (out green data-out)
        (in red write-address)
        (in red read-address)
        (signal sig-data)
        (signal sig-write-address)
        (signal sig-read-address))

       ,@(apply append
           (for (for (range width) (lambda (x) (+ x start-address)))
                (lambda (address)
                  (let ((state (strings->symbol "state" (number->string address))))
                    `((green ,state)
                      (decider (write-address data-in) ,state sig-write-address == ,address sig-data input-count)
                      (decider (write-address ,state) ,state sig-write-address != ,address sig-data input-count)
                      (decider (read-address ,state) data-out sig-read-address == ,address sig-data input-count)))))))))
