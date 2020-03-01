(load "circuits/util.scm")

(define read-binary-with
  (lambda (filepath reader)
    (call-with-input-file filepath
      (lambda (file)
        (letrec ((current-file-position-var 0)
                 (current-file-position (lambda () current-file-position-var))
                 (next-char (lambda ()
                              (set! (current-file-position-var) (list (+ 1 current-file-position-var)))
                              (read-char file)))
                 (next (lambda () (char->integer (next-char))))
                 (seek (lambda (pos)
                         (assert (<= (current-file-position) pos) "Unable to seek backwards.")
                         (if (eqv? (current-file-position) pos)
                           ()
                           (begin (next) (seek pos))))))
          (reader current-file-position next-char next seek))))))

(define read-elf-with
  (lambda (filepath header reader)
    (read-binary-with filepath
      (lambda (current-file-position next-char next seek)
        (let* ((little-endian (lookup header 'little-endian))
               (bytes-per-word (lookup header 'bytes-per-word))
               (next-n (lambda (num-bytes)
                         (letrec ((loop (if little-endian
                                          (lambda (n result)
                                            (if (eqv? n num-bytes)
                                              result
                                              (loop (+ n 1) (logior result (* (next) (expt 256 n))))))
                                          (lambda (n result)
                                            (if (eqv? n num-bytes)
                                              result
                                              (loop (+ n 1) (logior (* result 256) (next))))))))
                           (loop 0 0))))
               (next-word (lambda () (next-n bytes-per-word))))
          (reader current-file-position next-char next next-n next-word seek))))))

(define read-elf-header
  (lambda (filepath)
    (let ((header-head (read-binary-with filepath
                         (lambda (current-file-position next-char next seek)
                           (assert (eqv? #x7f (next)) "Expected 0x7f ELF.")
                           (assert (eqv? #\E (next-char)) "Expected 0x7f ELF.")
                           (assert (eqv? #\L (next-char)) "Expected 0x7f ELF.")
                           (assert (eqv? #\F (next-char)) "Expected 0x7f ELF.")
                           (let* ((n-bits-raw (next))
                                  (n-bits (if (eqv? n-bits-raw 1)
                                            32
                                            (begin (assert (eqv? n-bits-raw 2) "Only 32 or 64 bit ELFs are supported.")
                                                   64)))
                                  (bytes-per-word (/ n-bits 8)))
                             (let* ((endianness-raw (next))
                                    (little-endian (if (eqv? 1 endianness-raw)
                                                     #t
                                                     (begin (assert (eqv? 2 endianness-raw) "Unrecognized endianness.")
                                                            #f))))
                               `((n-bits ,n-bits)
                                 (bytes-per-word ,bytes-per-word)
                                 (little-endian ,little-endian))))))))
      (read-elf-with filepath header-head
        (lambda (current-file-position next-char next next-n next-word seek)
          (seek 6)
          (assert (eqv? 1 (next)) "Only ELF version 1 is supported.")
          (next) ; ignore e_ident[EI_OSABI]
          (next) ; ignore e_ident[EI_ABIVERSION]
          (next-n 7) ; ignore e_ident[EI_PAD]
          (next-n 2) ; ignore e_type
          (assert (eqv? (current-file-position) #x12) "Consumed an unexpected number of bytes so far.")
          (let ((isa (case (next-n 2)
                       ((#x00) 'isa-none)
                       ((#x02) 'isa-sparc)
                       ((#x03) 'isa-x86)
                       ((#x08) 'isa-mips)
                       ((#x14) 'isa-ppc)
                       ((#x16) 'isa-s390)
                       ((#x28) 'isa-arm)
                       ((#x2a) 'isa-superh)
                       ((#x32) 'isa-ia-64)
                       ((#x3e) 'isa-amd64)
                       ((#xb7) 'isa-aarch64)
                       ((#xf3) 'isa-riscv)
                       (else 'isa-unknown))))
            (assert (not (eqv? isa 'isa-unknown)) "Unknown ISA.")
            (assert (eqv? 1 (next-n 4)) "Only ELF version 1 is supported (and e_version differs from e_ident[EI_VERSION]).")
            (assert (eqv? (current-file-position) #x18) "Consumed an unexpected number of bytes so far.")
            (let ((entry-point (next-word))
                  (program-header-table-offset (next-word))
                  (section-header-table-offset (next-word)))
              (next-n 4) ; ignore e_flags
              (let ((header-size (next-n 2)))
                (if (eqv? (lookup header-head 'n-bits) 32)
                  (assert (eqv? header-size 52) "Unexpected header size for a 32 bit ELF.")
                  (assert (eqv? header-size 64) "Unexpected header size for a 64 bit ELF."))
                (let ((program-header-table-entry-size (next-n 2))
                      (program-header-table-entries (next-n 2))
                      (section-header-table-entry-size (next-n 2))
                      (section-header-table-entries (next-n 2))
                      (section-header-table-entry-with-section-names (next-n 2)))
                  (assert (eqv? (current-file-position) header-size) "Consumed an unexpected number of bytes so far.")
                  (if (eqv? (lookup header-head 'n-bits) 32)
                    (begin (assert (eqv? program-header-table-entry-size #x20) "Unexpected program header table entry size.")
                           (assert (eqv? section-header-table-entry-size #x28) "Unexpected section header table entry size."))
                    (begin (assert (eqv? program-header-table-entry-size #x38) "Unexpected program header table entry size.")
                           (assert (eqv? section-header-table-entry-size #x40) "Unexpected section header table entry size.")))
                  `((isa ,isa)
                    (entry-point ,entry-point)
                    (program-header-table-offset ,program-header-table-offset)
                    (section-header-table-offset ,section-header-table-offset)
                    (program-header-table-entry-size ,program-header-table-entry-size)
                    (program-header-table-entries ,program-header-table-entries)
                    (section-header-table-entry-size ,section-header-table-entry-size)
                    (section-header-table-entries ,section-header-table-entries)
                    (section-header-table-entry-with-section-names ,section-header-table-entry-with-section-names)
                    . ,header-head))))))))))

(define read-elf-program-headers
  (lambda (filepath header)
    (read-elf-with filepath header
      (lambda (current-file-position next-char next next-n next-word seek)
        (letrec ((program-header-size (lookup header 'program-header-table-entry-size))
                 (n-bits (lookup header 'n-bits))
                 (loop (lambda (num-remaining program-headers)
                         (if (eqv? num-remaining 0)
                           program-headers
                           (let ((start-pos (current-file-position))
                                 (segment-type (next-n 4))
                                 (p-flags-raw 0))
                             (if (eqv? n-bits 64) (set! (p-flags-raw) (list (next-n 4)))) ; Ignore p_flags (64 bit position).
                             (let ((file-address (next-word))
                                   (virt-mem-address (next-word))
                                   (phys-mem-address (next-word))
                                   (file-size (next-word))
                                   (mem-size (next-word)))
                               (if (eqv? n-bits 32) (set! (p-flags-raw) (list (next-n 4)))) ; Ignore p_flags (32 bit position).
                               (assert (eqv? 0 (logxor #x7 (logior #x7 p-flags-raw))) "Unknown p_flags.")
                               (let ((p-flags ())
                                     (pf-r 4)
                                     (pf-w 2)
                                     (pf-x 1))
                                 (if (< 0 (logand pf-r p-flags-raw)) (set! (p-flags) (list (cons 'r p-flags))))
                                 (if (< 0 (logand pf-w p-flags-raw)) (set! (p-flags) (list (cons 'w p-flags))))
                                 (if (< 0 (logand pf-x p-flags-raw)) (set! (p-flags) (list (cons 'x p-flags))))
                                 (next-word) ; Ignore p_align
                                 (assert (eqv? (current-file-position) (+ start-pos program-header-size)) "Consumed an unexpected number of bytes so far.")
                                 (loop (- num-remaining 1)
                                       (cons
                                         `((file-address ,file-address)
                                           (virt-mem-address ,virt-mem-address)
                                           (phys-mem-address ,phys-mem-address)
                                           (file-size ,file-size)
                                           (mem-size ,mem-size)
                                           (segment-type ,segment-type)
                                           (p-flags ,p-flags))
                                         program-headers)))))))))
          (seek (lookup header 'program-header-table-offset))
          (sort-by-key (lambda (entry) (lookup entry 'file-address)) (loop (lookup header 'program-header-table-entries) ())))))))

(define read-elf-program-segments
  (lambda (filepath)
    (let* ((header (read-elf-header filepath))
           (program-headers (read-elf-program-headers filepath header)))
      (read-elf-with filepath header
        (lambda (current-file-position next-char next next-n next-word seek)
          (letrec ((loop-over-segments
                     (lambda (result-segments-reversed remaining-program-headers)
                       (if (eq? remaining-program-headers ())
                         (reverse result-segments-reversed)
                         (letrec ((loop-over-words
                                    (lambda (result-words-reversed num-words-remaining)
                                      (if (eqv? num-words-remaining 0)
                                        (reverse result-words-reversed)
                                        (loop-over-words
                                          (cons (next-word) result-words-reversed)
                                          (- num-words-remaining 1)))))
                                  (program-header (car remaining-program-headers))
                                  (file-size (lookup program-header 'file-size))
                                  (mem-size (lookup program-header 'mem-size)))
                           (seek (lookup program-header 'file-address))
                           (let ((segment-words (loop-over-words () (/ file-size (lookup header 'bytes-per-word)))))
                             (if (eqv? file-size 0)
                               (begin (assert (> mem-size 0) "Empty segment.")
                                      (set! (segment-words) (list (replicate mem-size 0))))
                               (assert (eqv? file-size mem-size) "Memory and file sizes differ unexpectedly."))
                             (assert (eqv? (lookup program-header 'virt-mem-address) (lookup program-header 'phys-mem-address))
                                     "Virtual and physical memory addresses differ. I don't know what to do about that.")
                             (loop-over-segments (cons `((start-address ,(lookup program-header 'virt-mem-address))
                                                         (p-flags ,(lookup program-header 'p-flags))
                                                         (words ,segment-words))
                                                       result-segments-reversed)
                                                 (cdr remaining-program-headers))))))))
            (loop-over-segments () program-headers)))))))

; TODO: Deal with section overlaps properly!
(define assert-program-segments-not-overlapping
  (lambda (program-segments)
    (assert (> 2 (len program-segments))
            "You need to actually implement a check for overlapping segments instead of just hoping there's at most one segment.")))

; TODO: Some segments, like .bss, need to be writable!
(define assert-program-segments-not-writable
  (lambda (program-segments)
    (letrec ((loop (lambda (segments)
                     (let ((segment (car segments)))
                       (assert (not (memq 'w (lookup segment 'p-flags))) "You need to add support for writable program segments.")))))
      (loop program-segments))))

(define defpart-rom-of-elf
  (lambda (part-name filepath)
    (letrec ((program-segments (read-elf-program-segments filepath))
             (seg-part-name (lambda (index)
                              (sym-for-idx
                                (strings->symbol (symbol->string part-name) "-segment-") index)))
             (rom-segments
               (lambda (parts index remaining-segments)
                 (if (eq? () remaining-segments)
                   parts
                   (let ((segment (car remaining-segments)))
                     (rom-segments (cons `(defpart-rom
                                            ,(seg-part-name index)
                                            ,(/ (lookup segment 'start-address) 4)
                                            ,(map unsigned-literal-if-pos (lookup segment 'words)))
                                         parts)
                                   (+ 1 index)
                                   (cdr remaining-segments)))))))
      (assert-program-segments-not-overlapping program-segments)
      (assert-program-segments-not-writable program-segments)
      `(begin
         (defpart ,part-name
           ((in yellow address)
            (out yellow data-out)
            (signal address-signal)
            (signal data-out-signal))
           ,@(for (range (len program-segments))
               (lambda (index)
                 `(,(seg-part-name index) address data-out address-signal data-out-signal))))
         ,@(rom-segments () 0 program-segments)))))
