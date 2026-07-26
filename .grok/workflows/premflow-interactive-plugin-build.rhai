// Generated draft — Grok Build workflow template.
// Install: copy to .grok/workflows/premflow-interactive-plugin-build.rhai (or ~/.grok/workflows/).
// Requires Grok host APIs: agent(), phase(), log(), complete() — not rhai-host.
// Skill chain: parse-plan-and-context → external-tty-launch → journal-ensure-nonblocking → agent-skill-then-plugin

let meta = #{
    name: "premflow-interactive-plugin-build",
    description: "End-to-end development of interactive productivity commands (pomo, journal, coach) with external TTY, context labels and agent-skill integration",
    phases: [
        #{ title: "Core Engine", detail: "Pure session timer + CLI split" },
        #{ title: "Integration", detail: "External launch, journal ensure, coach from real signals" },
        #{ title: "Packaging", detail: "Roadmap, rename, remote plugin repo push" }
    ],
    when_to_use: "End-to-end development of interactive productivity commands (pomo, journal, coach) with external TTY, context labels and agent-skill integration",
};

phase("Core Engine");
let r0 = agent(
    "Follow skill 'parse-plan-and-context'. Pure session timer + CLI split. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "parse-plan-and-context",
        capability_mode: "read-only",
        output_schema: #{
            "type": "object",
            "required": ["done", "notes"],
            "properties": #{
                "done": #{ "type": "boolean" },
                "notes": #{ "type": "string" },
            },
        },
    },
);
if r0 == () || !r0.success {
    log("phase failed: parse-plan-and-context");
}


phase("Integration");
let r1 = agent(
    "Follow skill 'external-tty-launch'. External launch, journal ensure, coach from real signals. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "external-tty-launch",
        capability_mode: "read-write",
        output_schema: #{
            "type": "object",
            "required": ["done", "notes"],
            "properties": #{
                "done": #{ "type": "boolean" },
                "notes": #{ "type": "string" },
            },
        },
    },
);
if r1 == () || !r1.success {
    log("phase failed: external-tty-launch");
}


phase("Packaging");
let r2 = agent(
    "Follow skill 'journal-ensure-nonblocking'. Roadmap, rename, remote plugin repo push. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "journal-ensure-nonblocking",
        capability_mode: "read-write",
        output_schema: #{
            "type": "object",
            "required": ["done", "notes"],
            "properties": #{
                "done": #{ "type": "boolean" },
                "notes": #{ "type": "string" },
            },
        },
    },
);
if r2 == () || !r2.success {
    log("phase failed: journal-ensure-nonblocking");
}

complete(#{
    summary: "Finished workflow premflow-interactive-plugin-build",
    skill_chain: ["parse-plan-and-context","external-tty-launch","journal-ensure-nonblocking","agent-skill-then-plugin"],
});
