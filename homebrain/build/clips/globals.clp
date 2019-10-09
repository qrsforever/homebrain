;=================================================================
; date: 2018-06-14 19:11:27
; title: All Global Construct
; author: QRS
;=================================================================

;-----------------------------------------------------------------
;    Global varible
;       1. Use ?*var* to access the value of global varible
;       2. Use bind function to set the value of global varible.
;-----------------------------------------------------------------

; Salience Level Define
(defglobal
    ?*SALIENCE-HIGHEST*  =  10000
    ?*SALIENCE-HIGHER*   =   5000
    ?*SALIENCE-HIGH*     =   1000
    ?*SALIENCE-NORMAL*   =      0
    ?*SALIENCE-LOW*      =  -1000
    ?*SALIENCE-LOWER*    =  -5000
    ?*SALIENCE-LOWEST*   = -10000
)

; Log Level Define: must be consisence with defined in Log.h
(defglobal
    ?*LOG-LEVEL-ERROR*      = 1
    ?*LOG-LEVEL-WARNING*    = 2
    ?*LOG-LEVEL-INFO*       = 3
    ?*LOG-LEVEL-DEBUG*      = 4
    ?*LOG-LEVEL-TRACE*      = 5
)

; varibles: rule engine
(defglobal
    ?*RULE-TIMEOUT-MS*      = 3000
    ?*RULE-RETRY-COUNT*     = 1
)

;-----------------------------------------------------------------
;   Global Template
;-----------------------------------------------------------------

; timer event
(deftemplate timer-event
    (slot id)
)

;-----------------------------------------------------------------
;    Global Class
;-----------------------------------------------------------------

(defclass Context (is-a USER))

; Base Device Abstract
(defclass DEVICE (is-a USER) (role abstract)
    (slot ID     (visibility public) (type SYMBOL))
    (slot Class  (visibility public) (type SYMBOL) (access initialize-only))
    (slot UUID   (visibility public) (type STRING))
)

(defmessage-handler DEVICE init after ()
    (bind ?self:ID (instance-name-to-symbol (instance-name ?self)))
    (bind ?self:Class (class ?self))
)

; Sub Device define slot must specify facet: (visibility public)
(defmessage-handler DEVICE putData (?slot ?value)
    (bind ?old (nth$ 1 (dynamic-get ?slot)))
    (dynamic-put ?slot ?value ?old)
)

(defmessage-handler DEVICE getData (?slot)
    (return (nth$ 1 (dynamic-get ?slot)))
)

;-----------------------------------------------------------------
;    Global Rule
;-----------------------------------------------------------------

; show facts, ruels, instances and so on debug info
(defrule _rul-show-elem
    ?f <- (show ?elem)
  =>
    (retract ?f)
    (switch ?elem
        (case instances then    (instances))
        (case facts     then    (facts))
        (case templates then    (list-deftemplates))
        (case rules     then    (rules))
        (case agenda    then    (agenda))
        (case classes   then    (list-defclasses))
        (case globals   then    (show-defglobals))
        (case memory    then    (printout info "Memory Used:"(/ (mem-used) 131072)" MB" crlf))
        (default (printout warn "Unkown elem: " ?elem crlf))
    )
)

(defrule retract-start-homebrain
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (start-homebrain)
  =>
    (retract ?f)
)
; TODO: retract fact from cloud virtualmode
(defrule _rul-retract-virtualmode
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (virtualmode ?value)
  =>
    (retract ?f)
)

; (datetime (now)) from program
(defrule _rul-retract-datetime
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (datetime $?)
  =>
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
      then
        (facts)
    )
    (retract ?f)
)

; retract fact: (action-reponse id value)
(defrule _rul-retract-rule-response
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (rule-response ?id $?)
  =>
    (retract ?f)
)

(defrule _rul-remove-timer-event
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (remove-timer-event ?id)
  =>
    (retract ?f)
    (bind ?fact (nth$ 1 (find-fact ((?t timer-event)) (eq ?t:id ?id))))
    (if (neq ?fact nil)
     then
        (retract ?fact)
    )
)

; retract fact: (rule ruleID)
(defrule _rul-retract-rule
    (declare (salience ?*SALIENCE-LOWEST*))
    ?f <- (rule $?)
  =>
    (if (>= ?*LOG-LEVEL* ?*LOG-LEVEL-TRACE*)
      then
        (facts)
    )
    (retract ?f)
)
