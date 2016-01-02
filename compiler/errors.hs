module Errors where
import BasicTypes

-- Error handling

type ErrorContext = String

data CompileError = Redefine String
                  | MapLookupError String String -- name, index
                  | GraphLookupError Int
                  | DefaultError String
                  | InternalError String
                  | ParsingError String
                  | TypeError Type Type ErrorContext -- reqType actualType
                  | ReturnTypeError Type Type ErrorContext -- reqType actualType
                  | NeedsValueError ErrorContext -- void expression in value context
                  | LValueError ErrorContext
                  | LiteralError Type Int
                  | UndefinedVarError String ErrorContext
                  | UndefinedMethodError String ErrorContext
                  | MethodArgsError String Int ErrorContext
                  | VoidCastError String
                  | NoReturnError ErrorContext
                  | NoMainError
                  deriving Show

type ThrowsError = Either CompileError

maybeToError :: e -> Maybe a -> Either e a
maybeToError _ (Just x)  = Right x
maybeToError c (Nothing) = Left c


isError :: ThrowsError a -> Bool
isError (Right _) = False
isError (Left _) = True
