// Generated draft — Grok Build workflow template.
// Install: copy to .grok/workflows/issue-to-preventive-skill.rhai (or ~/.grok/workflows/).
// Requires Grok host APIs: agent(), phase(), log(), complete() — not rhai-host.
// Skill chain: session-unit-order → dual-copy-skill-publish

let meta = #{
    name: "issue-to-preventive-skill",
    description: "Convert a systemd race bug report into a regression doc plus composable skill, then publish dual copies",
    phases: [
        #{ title: "Diagnose", detail: "Audit units and confirm Linger + graphical-session race" },
        #{ title: "Fix", detail: "Edit unit and verify boot" },
        #{ title: "Extract", detail: "Write REGRESSION.md and SKILL.md" },
        #{ title: "Publish", detail: "Dual-copy to project + skills repo with Omarchy note" }
    ],
    when_to_use: "Convert a systemd race bug report into a regression doc plus composable skill, then publish dual copies",
};

phase("Diagnose");
let r0 = agent(
    "Follow skill 'session-unit-order'. Audit units and confirm Linger + graphical-session race. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "session-unit-order",
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
    log("phase failed: session-unit-order");
}


phase("Fix");
let r1 = agent(
    "Follow skill 'dual-copy-skill-publish'. Edit unit and verify boot. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "dual-copy-skill-publish",
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
    log("phase failed: dual-copy-skill-publish");
}


phase("Extract");
let r2 = agent(
    "Follow skill 'Extract'. Write REGRESSION.md and SKILL.md. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "Extract",
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
    log("phase failed: Extract");
}


phase("Publish");
let r3 = agent(
    "Follow skill 'Publish'. Dual-copy to project + skills repo with Omarchy note. Use read-only tools first when mapping; "
        + "then apply edits. Report {done: bool, notes: string}.",
    #{
        label: "Publish",
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
if r3 == () || !r3.success {
    log("phase failed: Publish");
}

complete(#{
    summary: "Finished workflow issue-to-preventive-skill",
    skill_chain: ["session-unit-order","dual-copy-skill-publish"],
});
