//////////////////////////////////////////////////////////////////////////////
//
// Editor - Treap
// listener/winapp/ed_BinTree.h
//
// Copyright (C) 1996-2007 by Project Vogue.
// Written by Yoshifumi "VOGUE" INOUE. (yosi@msn.com)
//
// @(#)$Id: //proj/evcl3/mainline/listener/winapp/ed_BinTree.h#1 $
//
#if !defined(INCLUDE_edit_binary_tree_h)
#define INCLUDE_edit_binary_tree_h

extern int TreapRandom();

template<class Node_>
class BinaryTree
{
    public: class NodeBase
    {
        friend class BinaryTree;

        private:   int      m_iPriority;
        protected: Node_*   m_pLeft;
        protected: Node_*   m_pRight;

        public: NodeBase() :
            m_iPriority(TreapRandom()),
            m_pLeft(NULL),
            m_pRight(NULL) {}

        // [D]
        private: Node_* deleteNode()
        {
            if (NULL == m_pLeft)
            {
                return m_pRight;
            }

            if (NULL == m_pRight)
            {
                return m_pLeft;
            }

            if (m_pLeft->m_iPriority < m_pRight->m_iPriority)
            {
                Node_* pTemp    = rotateRight();
                pTemp->m_pRight = deleteNode();
                return pTemp;
            }

            {
                Node_* pTemp   = rotateLeft();
                pTemp->m_pLeft = deleteNode();
                return pTemp;
            }
        } // deleteNode

        public: Node_* GetLeft()   const { return m_pLeft; }
        public: Node_* GetParent() const { return m_pParent; }
        public: Node_* GetRight()  const { return m_pRight; }

        // [R]
        private: Node_* rotateLeft()
        {
            Node_* pTemp    = m_pRight;
            m_pRight        = m_pRight->m_pLeft;
            pTemp->m_pLeft  = static_cast<Node_*>(this);
            return pTemp;
        } // rotateLeft

        private: Node_* rotateRight()
        {
            Node_* pTemp    = m_pLeft;
            m_pLeft         = m_pLeft->m_pRight;
            pTemp->m_pRight = static_cast<Node_*>(this);
            return pTemp;
        } // rotateRight
    }; // NodeBase

    private: Node_* m_pRoot;

    // ctor
    public: BinaryTree() :
        m_pRoot(NULL) {}

    // [D]
    public: void Delete(Node_* pNode)
    {
        m_pRoot = deleteAux(m_pRoot, pNode);
    } // Delete

    private: static Node_* deleteAux(Node_* pTree, Node_* pNode)
    {
        // pNode must be in pTree.
        ASSERT(NULL != pTree);

        if (pTree == pNode)
        {
            return pTree->deleteNode();
        }

        if (pNode->Compare(pTree) < 0)
        {
            pTree->m_pLeft = deleteAux(pTree->m_pLeft, pNode);
        }
        else
        {
            pTree->m_pRight = deleteAux(pTree->m_pRight, pNode);
        }

        return pTree;
    } // deleteAux

    // [G]
    public: Node_* GetRoot() const { return m_pRoot; }

    // [I]
    public: Node_* Insert(Node_* pNode)
    {
        m_pRoot = insertAux(m_pRoot, pNode);
        return pNode;
    } // Insert

    private: static Node_* insertAux(Node_* pTree, Node_* pNode)
    {
        if (NULL == pTree)
        {
            return pNode;
        }

        if (pNode->Compare(pTree) < 0)
        {
            Node_* pLeft = insertAux(pTree->m_pLeft, pNode);
            pTree->m_pLeft = pLeft;
            if (pLeft->m_iPriority > pTree->m_iPriority)
            {
                pTree = pTree->rotateRight();
            }
            return pTree;
        }

        {
            Node_* pRight = insertAux(pTree->m_pRight, pNode);
            pTree->m_pRight = pRight;
            if (pRight->m_iPriority > pTree->m_iPriority)
            {
                pTree = pTree->rotateLeft();
            }
            return pTree;
        }
    } // Insert
}; // BinaryTree

#endif //!defined(INCLUDE_edit_binary_tree_h)
