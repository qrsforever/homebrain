(defrule rul-001-gradlight-down-service
    (datetime ?clock $?ox)
    ?svr <- (service
        (name ?name &:(eq ?name gradlight-down))
        (skey ?key) (state ?st) (saves $?saves)
        (args ?light ?cnt ?timelen $?oy)
        (ntime ?tm &:(or (> ?clock ?tm) (neq ?st running)))
    )
    ?ins <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ?light))
        (brightness ?brg ?brg-old)
    )
  =>
    (bind ?c (create-rule-context (sym-cat rul-001-gradlight-down-service : ?key)))
    (logi "service: " ?name ", skey: " ?key ", state: " ?st)
    (if (eq ?st stop)
     then
        (return)
    )
    (if (eq ?st start)
     then
        (if (< ?timelen ?cnt)
         then
            (send ?c act-control ?did brightness 0)
            (return)
        )
        (bind ?deltatime (integer (/ ?timelen ?cnt)))
        (bind ?deltabrg (integer (/ ?brg ?cnt)))
        (modify ?svr (state running) (ntime (+ ?clock ?deltatime)) (saves ?clock ?brg ?deltatime ?deltabrg))
        (return)
    )
    (if (<= ?brg 0)
     then
        (send ?c act-control ?did value 0)
        (assert (stop-service (sym-cat ?name : ?key)))
        (return)
    )
    (bind ?starttime (nth$ 1 ?saves))
    (bind ?startbrg (nth$ 2 ?saves))
    (bind ?deltatime (nth$ 3 ?saves))
    (bind ?co (integer (/ (- ?clock ?starttime) ?deltatime)))
    (if (> ?co 0)
     then
        (bind ?deltabrg (nth$ 4 ?saves))
        (bind ?target (- ?startbrg (* ?deltabrg ?co)))
        (if (> ?target 0)
         then
            (send ?c act-control ?did brightness ?target)
         else
            (send ?c act-control ?did brightness 0)
        )
        (modify ?svr (ntime (+ ?clock ?deltatime)))
    )
)
