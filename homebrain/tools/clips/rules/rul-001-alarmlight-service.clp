(defrule rul-001-alarmlight-service
    (datetime ?clock $?ox)
    ?svr <- (service (name ?name &:(eq ?name alarmlight))
        (skey ?key) (state ?st)
        (args ?etime ?light ?hue ?sat $?oy)
        (ntime ?tm &:(or (> ?clock ?tm) (neq ?st running)))
    )
    ?ins <- (object (is-a oic.d.light)
        (ID ?did &:(eq ?did ?light))
        (value ?val ?val-old)
    )
  =>
    (bind ?c (create-rule-context (sym-cat rul-001-alarmlight-service : ?key)))
    (logi "service: " ?name ", skey: " ?key ", state: " ?st)
    (if (eq ?st stop)
     then
        (send ?c act-control ?did value 1)
        (return)
    )
    (if (eq ?st start)
     then
        (send ?c act-control ?did value 1)
        (send ?c act-control ?did brightness 100)
        (send ?c act-control ?did hue ?hue)
        (send ?c act-control ?did saturation ?sat)
        (modify ?svr (state running))
        (return)
    )
    (if (= ?val 1)
     then
        (send ?c act-control ?did value 0)
     else
        (send ?c act-control ?did value 1)
    )
    (if (or (= ?etime -1) (< ?clock ?etime))
     then
        (modify ?svr (ntime (+ ?clock 500)))
     else
	    (assert (stop-service (sym-cat ?name : ?key)))     
    )
)
