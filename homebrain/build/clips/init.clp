;=================================================================
; date: 2018-06-04 20:21:39
; title: Unit Test
; author: QRS
;=================================================================

(defglobal
    ?*LOG-LEVEL* = (get-debug-level)
    ?*ROOT-DIR* = (get-root-dir)
    ?*CLIPS-DIRS* = (create$ classes rules temlates)
    ?*TEST-CASE* = FALSE
    ?*START-TIME* = (now)
)

(deffunction resolve-file (?file ?ispath)
    (if (eq ?ispath FALSE)
     then
        (foreach ?d ?*CLIPS-DIRS*
            (bind ?path (str-cat ?*ROOT-DIR* "/" ?d "/" ?file))
            (if (open ?path fd)
             then
                (close fd)
                (return ?path)
            )
        )
     else
        (if (open ?file fd)
         then
            (close fd)
            (return ?file)
        )
    )
    (printout error "Not found file: " ?file crlf)
    (return FALSE)
)

(load* (str-cat ?*ROOT-DIR* "/" globals.clp))
(load* (str-cat ?*ROOT-DIR* "/" utils.clp))
(load* (str-cat ?*ROOT-DIR* "/" rulecontext.clp))
(load* (str-cat ?*ROOT-DIR* "/" scenecontext.clp))
(load* (str-cat ?*ROOT-DIR* "/" devices.clp))

(defrule load-files
    (init)
  =>
    ; (bind ?templfs (get-files ?*TYPE-TEM-FILE*))
    ; (bind ?clsesfs (get-files ?*TYPE-CLS-FILE*))
    ; (bind ?rulesfs (get-files ?*TYPE-RUL-FILE*))

    ; (load-files ?templfs)
    ; (load-files ?clsesfs)
    ; (load-files ?rulesfs)

    (assert (load-finished))
)

(defrule init-finished
    ?f1 <- (init)
    ?f2 <- (load-finished)
  =>
    (retract ?f1)
    (retract ?f2)
    ; Simulate Test Case
    ; (reset)
    (if ?*TEST-CASE*
     then
        (batch* (str-cat ?*ROOT-DIR* "/" testcase.clp))
        (assert (test-suite init))
    )
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
     then
        (printout trace ">>>>>> list globals:" crlf)
        (show-defglobals)
        (printout trace ">>>>>> list defclasses:" crlf)
        (list-defclasses)
        (printout trace ">>>>>> list defrules:" crlf)
        (list-defrules)
        (printout trace ">>>>>> list deffunctions:" crlf)
        (list-deffunctions)
        (printout trace crlf)
    )
    (seed (integer (time)))

    ; call RuleEngineCore function
    (init-finished)

    (assert (start-homebrain))
)

; TODO
(defrule init-micro-services
    ?f <- (load-micro-services)
  =>
    (printout trace ">>>>>> load micro services <<<<<<<" crlf)
    (retract ?f)

    (load* (str-cat ?*ROOT-DIR* "/ms/" rul-ms-autolight-service.clp))
    (load* (str-cat ?*ROOT-DIR* "/ms/" rul-ms-gradlight-service.clp))
    (load* (str-cat ?*ROOT-DIR* "/ms/" rul-ms-sosalarm-service.clp))
)

;-----------------------------------------------------------------
;    reset: remove all activations, facts, instances, then
;           1. assign globals,
;           2. assert all facts in deffacts
;           3. create all intances in definstances
;           4. set current moudle to the MAIN
;   TODO: call it by program
;-----------------------------------------------------------------
; (reset)
