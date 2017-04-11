//
// Copyright 2016 Timo Kloss
//
// This file is part of LowRes NX.
//
// LowRes NX is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LowRes NX is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with LowRes NX.  If not, see <http://www.gnu.org/licenses/>.
//

#import "ViewController.h"
#import "core.h"
#import "RendererViewController.h"
#import "AppDelegate.h"

@interface ViewController () <UIKeyInput>
@property (nonatomic) RendererViewController *rendererViewController;
@property (nonatomic) BOOL isKeyboardActive;
@end

@implementation ViewController {
    struct Core *_core;
}

- (void)viewDidLoad
{
    AppDelegate *delegate = (AppDelegate *)[UIApplication sharedApplication].delegate;
    struct Core *core = [delegate getCore];
    _core = core;
    [self.rendererViewController setCore:core];
}

- (void)viewDidAppear:(BOOL)animated
{
    [super viewDidAppear:animated];
    self.isKeyboardActive = YES;
}

- (BOOL)prefersStatusBarHidden
{
    return YES;
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    if ([segue.identifier isEqualToString:@"renderer"])
    {
        self.rendererViewController = segue.destinationViewController;
    }
}

- (void)setIsKeyboardActive:(BOOL)active
{
    _isKeyboardActive = active;
    if (active)
    {
        [self becomeFirstResponder];
    }
    else
    {
        [self resignFirstResponder];
    }
}

- (BOOL)canBecomeFirstResponder
{
    return self.isKeyboardActive;
}

- (UITextAutocorrectionType)autocorrectionType
{
    return UITextAutocorrectionTypeNo;
}

- (UITextSpellCheckingType)spellCheckingType
{
    return UITextSpellCheckingTypeNo;
}

- (UIKeyboardAppearance)keyboardAppearance
{
    return UIKeyboardAppearanceDark;
}

- (BOOL)hasText
{
    return YES;
}

- (void)insertText:(NSString *)text
{
    if (text.length > 0)
    {
        unichar key = [text.uppercaseString characterAtIndex:0];
        if (key < 127)
        {
            _core->machine.ioRegisters.key = key;
        }
    }
}

- (void)deleteBackward
{
    _core->machine.ioRegisters.key = '\b';
}

// this is from UITextInput, needed because of crash on iPhone 6 keyboard (left/right arrows)
- (UITextRange *)selectedTextRange
{
    return nil;
}

@end
