(load "util.scm")

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
