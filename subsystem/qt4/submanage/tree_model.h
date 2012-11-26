/**
 * \file tree_model.h
 * \author Dennis Terhorst
 * \date Mon Jul 28 14:24:17 CEST 2008
 */
#ifndef TREE_MODEL_H
#define TREE_MODEL_H

#include <QAbstractItemModel>
#include "tree_t.h"

class tree_model : public QAbstractItemModel {
	private:
		tree_t* tree;
	public:
		tree_model(QObject * parent = 0) : QAbstractItemModel(parent) {
			tree = NULL;
		}
		~tree_model() {
			delete(tree);
		}

	// functions needed from a QAbstractItemModel subclass:
		index();
		parent();
		rowCount();
		columnCount();
		data();
	// optional QAbstractItemModel virtuals:
		hasChildren()
		// for editable graphs
		flags();
		setData();		// emit dataChanged()
		headerData();
		setHeaderData();	// emit headerDataChanged()


	// bla:
		//ItemFlag()  returns if element dragable
	signals:
		dataChanged()
		headerDataChanged()
		layoutChanged()
}

#endif //ndef TREE_MODEL_H
