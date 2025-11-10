#ifndef H_REFERENCE_ENTRY
#define H_REFERENCE_ENTRY

#include <QString>
#include <QList>

class enum ReferenceType {
	book,
	paper,
	article,
	blogPost,
	blog,
	video,
	podcast
};

class ReferenceEntry {
public:
	ReferenceEntry();
	ReferenceEntry(
		ReferenceType type,
		QString& title,
		QString& author,
		QString& publishDate,
		QString& url,
		QList<QString> notes
	);
	~ReferenceEntry();

	// Type.
	const ReferenceType type const { return m_type; }
	ReferenceType type() { return m_type; }
	// Title.
	const QString& title() const { return m_title; }
	QString& title() { return m_title; }
	// Author.
	const QString& author() const { return m_author; }
	QString& author() { return m_author; }
	// Publishing date.
	const QString& publishDate() const { return m_publishDate; }
	QString& publishDate() { return m_publishDate; }
	// URL.
	const QString& url() const { return m_url; }
	QString& url() { return m_url; }
	// Notes.
	const QList<QString> notes() const { return m_notes; };
	int addNote(QString& note);
	void removeNote(int index);

private:
	ReferenceType m_type;
	QString m_title;
	QString m_author;
	QString m_publishingDate;
	QString m_url;
	QList<QString> m_notes;
};

#endif

