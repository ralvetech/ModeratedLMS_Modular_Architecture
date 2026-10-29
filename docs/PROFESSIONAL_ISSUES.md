# Professional, Legal and Ethical Issues

## Privacy and data protection
The LMS stores learner identities, enrolments, submissions, marks and feedback. These records can materially affect educational opportunities and must be treated as confidential. The school should identify the applicable Lesotho data-protection requirements and institutional policies with a qualified authority; this project does not claim legal certification. Collect only fields needed for teaching and assessment, define a retention schedule, restrict database and backup access, and provide a process for correcting inaccurate personal data.

## Assessment integrity and accountability
Marks must not be exposed before HoD moderation. Every mark change should record the actor, time, previous value, new value and reason. Audit rows should be append-only to ordinary users and access to audit history should be limited to authorised staff. A production implementation should write the mark and audit record in one database transaction so a mark cannot change without its audit event.

## Fairness of late penalties
A fixed penalty may disadvantage students affected by illness, disability, emergencies, connectivity or circumstances outside their control. The brief requires individual extensions and a late penalty; the system should therefore support documented extensions, transparent effective deadlines, written feedback and a review/appeal route. The exact definition of “day late,” treatment of weekends, partial days, approved accommodations and the 10%-of-available-mark calculation must be agreed with the school and documented before deployment.

## Security
Passwords are stored as salted scrypt-derived hashes rather than plaintext. The current demo accounts and shared initial password are strictly for demonstration and must not be used with real student records. A production version should enforce password change, rate-limit failed login attempts, protect database files and backups, minimise privilege, and have a documented incident-response process. Local access to the SQLite file can bypass GUI controls, so OS permissions and physical security matter.

## Accessibility and inclusion
A desktop GUI should support keyboard navigation, visible focus, meaningful labels, sufficient contrast, scalable text and clear error messages. Conduct tests with users who have different access needs; Qt widgets alone do not guarantee accessibility. Feedback and deadlines should be presented in plain language.

## Professional responsibility
The developer should communicate limitations honestly, keep source control and test evidence, avoid fabricating test results, and obtain approval for requirements that affect grades. Before deployment, the school should appoint a data owner, define support responsibilities and establish backup/restore procedures. The prototype must not be represented as production-ready until security, usability, performance and recovery criteria are measured and passed.
