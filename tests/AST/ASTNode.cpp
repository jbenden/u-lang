/**
 * The U Programming Language
 *
 * Copyright 2018 Joseph Benden
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * \author Joseph W. Benden
 * \copyright (C) 2018 Joseph Benden
 * \license apache2
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <u-lang/AST/ASTContext.hpp>
#include <u-lang/AST/ASTNode.hpp>
#include <u-lang/u.hpp>

using namespace u;

TEST(ASTNode, InitiallyNoMemoryAllocated) // NOLINT
{
  ASTContext ctx;

  EXPECT_EQ(ctx.getASTAllocatedMemory(), 0u);
}

TEST(ASTNode, MemoryIsAllocatedViaBumpPtrAllocator) // NOLINT
{
  ASTContext ctx;

  auto unitAST = new(ctx) UnitAST;

  EXPECT_TRUE(unitAST != nullptr);
  EXPECT_EQ(unitAST->getKind(), ASTNode::UnitASTKind);
  EXPECT_GT(ctx.getASTAllocatedMemory(), 0u);
}

TEST(ASTNode, EmptyUnitASTHasNoIncomingNodes) // NOLINT
{
  ASTContext ctx;

  auto unitAST = new(ctx) UnitAST;

  EXPECT_TRUE(unitAST != nullptr);
  EXPECT_TRUE(unitAST->incoming_begin() == unitAST->incoming_end());
}

TEST(ASTNode, EmptyUnitASTHasNoOutgoingNodes) // NOLINT
{
  ASTContext ctx;

  auto unitAST = new(ctx) UnitAST;

  EXPECT_TRUE(unitAST != nullptr);
  EXPECT_TRUE(unitAST->outgoing_begin() == unitAST->outgoing_end());

  // cast to base class and attempt access to outgoing vertices.
  auto astNode = dynamic_cast<ASTNode*>(unitAST);
  EXPECT_TRUE(astNode->outgoing_begin() == astNode->outgoing_end());
}

TEST(ASTNode, EmptyUnitASTISA) // NOLINT
{
  ASTContext ctx;

  auto unitAST = new(ctx) UnitAST;

  EXPECT_TRUE(unitAST != nullptr);
  EXPECT_TRUE(llvm::isa<UnitAST>(unitAST));
}
