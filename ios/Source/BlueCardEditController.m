//
//  BlueCardEditController.m
//  Follow
//
//  Created by Erich Ocean on 7/29/16.
//  Copyright © 2016 Xy Group Ltd. All rights reserved.
//

#import "BlueCardEditController.h"

#import "BlueApp.h"

#import <TOCropViewController/TOCropViewController.h>

@interface BlueCardEditController () <UITextFieldDelegate, UIImagePickerControllerDelegate, UINavigationControllerDelegate, TOCropViewControllerDelegate>

@property (nonatomic, assign) BOOL editingAvatar;
@property (nonatomic, assign) BOOL isEditingAgain;

@end

@implementation BlueCardEditController

- (void) viewDidLoad
{
    [super viewDidLoad];

    if (APP_DELEGATE.isUpdatingCard) {
        UIBarButtonItem *cancelButton = [[UIBarButtonItem alloc] initWithBarButtonSystemItem: UIBarButtonSystemItemCancel target: self action: @selector(cancel:)];
        [[self navigationItem] setLeftBarButtonItem: cancelButton];
        self.navigationController.navigationItem.hidesBackButton = NO;
    }
    
    self.fullNameTextField.delegate = self;
    self.locationTextField.delegate = self;

    if (!APP_DELEGATE.networks) {
        APP_DELEGATE.networks = [NSMutableDictionary dictionaryWithCapacity: 3];
    }

    // Need to set up the properties of our text fields.
    if (APP_DELEGATE.fullName) {
        self.fullNameTextField.text = APP_DELEGATE.fullName;
    }

    if (APP_DELEGATE.location) {
        self.locationTextField.text = APP_DELEGATE.location;
    }

    if (APP_DELEGATE.bio) {
        self.bioTextView.text = APP_DELEGATE.bio;
    }

    self.avatarImageView.clipsToBounds = YES;
    self.avatarImageView.layer.cornerRadius = 78; // this value vary as per your desire
    self.avatarImageView.layer.borderColor = [UIColor whiteColor].CGColor;
    self.avatarImageView.layer.borderWidth = 1.0;

    if (APP_DELEGATE.croppedAvatar) {
        self.avatarImageView.image = APP_DELEGATE.croppedAvatar;

    } else if (APP_DELEGATE.savedAvatar) {
        self.avatarImageView.image = APP_DELEGATE.savedAvatar;
    }

    if (APP_DELEGATE.croppedCoverPhoto) {
        self.coverPhotoImageView.image = APP_DELEGATE.croppedCoverPhoto;

    } else if (APP_DELEGATE.savedCoverPhoto) {
        self.coverPhotoImageView.image = APP_DELEGATE.savedCoverPhoto;
    }

    [[NSNotificationCenter defaultCenter] addObserver: self selector: @selector(updateBio:) name: @"BioDidChange" object: nil];
}

- (void) cancel: (id) sender
{
    [self dismissViewControllerAnimated: YES completion: nil];
}

- (void) updateBio: (NSNotification *) note
{
    self.bioTextView.text = APP_DELEGATE.bio;
}

- (void)
tableView:             (UITableView *) tableView
willDisplayFooterView: (UIView *)      view
forSection:            (NSInteger)     section
{
    // Set the background color of the View
    view.tintColor = [UIColor blackColor];
}

- (IBAction) editAvatar: sender
{
    self.editingAvatar = YES;

    if (APP_DELEGATE.originalAvatar) {
        // Need to ask if we should get a new image, or edit the existing one.
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Avatar"
                                                      message:                  @"What would you like to do?"
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *selectImageAction = [UIAlertAction actionWithTitle: @"Choose a new image"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction * _Nonnull action) {
            [self selectImage];
        }];

        UIAlertAction *editExistingAction = [UIAlertAction actionWithTitle: @"Edit the current image"
                                                           style:           UIAlertActionStyleDefault
        handler: ^(UIAlertAction * _Nonnull action) {
            self.isEditingAgain = YES;
            [self editImage];
        }];

        UIAlertAction *removeExistingAction = [UIAlertAction actionWithTitle: @"Remove the current image"
                                                             style:           UIAlertActionStyleDefault
        handler: ^(UIAlertAction * _Nonnull action) {
            APP_DELEGATE.originalAvatar = nil;
            APP_DELEGATE.croppedAvatar = nil;
            self.avatarImageView.image = [UIImage imageNamed: @"missing-avatar"];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: selectImageAction];
        [alert addAction: editExistingAction];
        [alert addAction: removeExistingAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];

    } else {
        self.isEditingAgain = NO;
        [self selectImage];
    }
}

- (IBAction) editCoverPhoto: sender
{
    self.editingAvatar = NO;


    if (APP_DELEGATE.originalCoverPhoto) {
        // Need to ask if we should get a new image, or edit the existing one.
        UIAlertController *alert = [UIAlertController alertControllerWithTitle: @"Cover Photo"
                                                      message:                  @"What would you like to do?"
                                                      preferredStyle:           UIAlertControllerStyleActionSheet];

        UIAlertAction *selectImageAction = [UIAlertAction actionWithTitle: @"Choose a new image"
                                                          style:           UIAlertActionStyleDefault
                                                          handler:
        ^(UIAlertAction * _Nonnull action) {
            [self selectImage];
        }];

        UIAlertAction *editExistingAction = [UIAlertAction actionWithTitle: @"Edit the current image"
                                                           style:           UIAlertActionStyleDefault
        handler: ^(UIAlertAction * _Nonnull action) {
            self.isEditingAgain = YES;
            [self editImage];
        }];

        UIAlertAction *removeExistingAction = [UIAlertAction actionWithTitle: @"Remove the current image"
                                                             style:           UIAlertActionStyleDefault
        handler: ^(UIAlertAction * _Nonnull action) {
            APP_DELEGATE.originalCoverPhoto = nil;
            APP_DELEGATE.croppedCoverPhoto = nil;
            self.coverPhotoImageView.image = [UIImage imageNamed: @"default-cover-photo"];
        }];

        UIAlertAction *cancelAction = [UIAlertAction actionWithTitle: @"Cancel" style: UIAlertActionStyleCancel handler: nil];

        [alert addAction: selectImageAction];
        [alert addAction: editExistingAction];
        [alert addAction: removeExistingAction];
        [alert addAction: cancelAction];

        [self presentViewController: alert animated: YES completion: nil];

    } else {
        self.isEditingAgain = NO;
        [self selectImage];
    }
}

- (void) selectImage
{
    UIImagePickerController *picker = [[UIImagePickerController alloc] init];

    picker.delegate = self;
    picker.allowsEditing = NO;
    picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;

    [self presentViewController:picker animated: YES completion: NULL];
}

- (void) editImage
{
    TOCropViewController *cropViewController = nil;

    if (self.editingAvatar) {
        cropViewController = [[TOCropViewController alloc] initWithCroppingStyle: TOCropViewCroppingStyleCircular image: APP_DELEGATE.originalAvatar];

        if (self.isEditingAgain) {
            cropViewController.imageCropFrame = APP_DELEGATE.croppedAvatarRect;
            cropViewController.angle = APP_DELEGATE.croppedAvatarAngle;
        }

    } else {
        cropViewController = [[TOCropViewController alloc] initWithImage: APP_DELEGATE.originalCoverPhoto];
        cropViewController.aspectRatioPreset = TOCropViewControllerAspectRatioPresetCustom;
        cropViewController.customAspectRatio = CGSizeMake(2.0, 3.0);
        cropViewController.aspectRatioLockEnabled = YES;

        if (self.isEditingAgain) {
            cropViewController.imageCropFrame = APP_DELEGATE.croppedCoverPhotoRect;
            cropViewController.angle = APP_DELEGATE.croppedCoverPhotoAngle;
        }
    }

    cropViewController.delegate = self;

    [self presentViewController: cropViewController animated: YES completion: nil];
}

#pragma mark Image Resize Utility

// See http://stackoverflow.com/a/2658801
- (UIImage *)
imageWithImage: (UIImage *) image
scaledToSize:   (CGSize)    newSize
{
    //UIGraphicsBeginImageContext(newSize);
    // In next line, pass 0.0 to use the current device's pixel scaling factor (and thus account for Retina resolution).
    // Pass 1.0 to force exact pixel size.
    UIGraphicsBeginImageContextWithOptions(newSize, NO, 1.0);
    [image drawInRect: CGRectMake(0, 0, newSize.width, newSize.height)];
    UIImage *newImage = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();

    return newImage;
}

#pragma mark TOCropViewControllerDelegate

- (void)
cropViewController:     (TOCropViewController *) cropViewController
didCropToCircularImage: (UIImage *)              image
withRect:               (CGRect)                 cropRect
angle:                  (NSInteger)              angle
{
    // 'image' is the newly cropped, circular version of the original image
    if (self.editingAvatar) {
        APP_DELEGATE.croppedAvatar = [self imageWithImage: image scaledToSize: CGSizeMake(312, 312)];
        self.avatarImageView.image = APP_DELEGATE.croppedAvatar;
        APP_DELEGATE.croppedAvatarRect = cropRect;
        APP_DELEGATE.croppedAvatarAngle = angle;
    }

    [cropViewController dismissViewControllerAnimated: YES completion: NULL];
}

- (void)
cropViewController: (TOCropViewController *) cropViewController
didCropToImage:     (UIImage *)              image
withRect:           (CGRect)                 cropRect
angle:              (NSInteger)              angle
{
   // 'image' is the newly cropped, aspect-ratio version of the original image
    if (!self.editingAvatar) {
        APP_DELEGATE.croppedCoverPhoto = [self imageWithImage: image scaledToSize: CGSizeMake(580, 870)];;
        self.coverPhotoImageView.image = APP_DELEGATE.croppedCoverPhoto;
        APP_DELEGATE.croppedCoverPhotoRect = cropRect;
        APP_DELEGATE.croppedCoverPhotoAngle = angle;
    }

    [cropViewController dismissViewControllerAnimated: YES completion: NULL];
}

- (void)
cropViewController: (TOCropViewController *) cropViewController
didFinishCancelled: (BOOL)                   cancelled
{
    // We need to remove the image if we didn't end up using it.
    if (self.editingAvatar) {
        if (self.avatarImageView.image != APP_DELEGATE.croppedAvatar) {
            APP_DELEGATE.originalAvatar = nil;
            APP_DELEGATE.croppedAvatar = nil;
            self.avatarImageView.image = [UIImage imageNamed: @"missing-avatar"];
        }

    } else {
        if (self.coverPhotoImageView.image != APP_DELEGATE.croppedCoverPhoto) {
            APP_DELEGATE.originalCoverPhoto = nil;
            APP_DELEGATE.croppedCoverPhoto = nil;
            self.coverPhotoImageView.image = [UIImage imageNamed: @"default-cover-photo"];
        }
    }

    [cropViewController dismissViewControllerAnimated: YES completion: NULL];
}

#pragma mark UIImagePickerControllerDelegate

- (void)
imagePickerController: (UIImagePickerController *) picker
didFinishPickingImage: (UIImage *)                 img
editingInfo:           (NSDictionary *)            editInfo
{
    UIImage *chosenImage  = [editInfo objectForKey: UIImagePickerControllerOriginalImage];

    if (self.editingAvatar) {
        APP_DELEGATE.originalAvatar = chosenImage ? chosenImage : img;

    } else {
        APP_DELEGATE.originalCoverPhoto = chosenImage ? chosenImage : img;
    }

    [picker dismissViewControllerAnimated: YES completion:^{
        [self editImage];
    }];
}

- (void)
imagePickerControllerDidCancel: (UIImagePickerController *) picker
{
    // We need to remove the image if we didn't end up using it.
    if (self.editingAvatar) {
        if (self.avatarImageView.image != APP_DELEGATE.croppedAvatar) {
            APP_DELEGATE.originalAvatar = nil;
            APP_DELEGATE.croppedAvatar = nil;
            self.avatarImageView.image = [UIImage imageNamed: @"missing-avatar"];
        }

    } else {
        if (self.coverPhotoImageView.image != APP_DELEGATE.croppedCoverPhoto) {
            APP_DELEGATE.originalCoverPhoto = nil;
            APP_DELEGATE.croppedCoverPhoto = nil;
            self.coverPhotoImageView.image = [UIImage imageNamed: @"default-cover-photo"];
        }
    }

    [picker dismissViewControllerAnimated: YES completion: NULL];
}

#pragma mark UITextFieldDelegate

- (BOOL) textFieldShouldReturn: (UITextField *) textField
{
    if (textField == self.fullNameTextField) {
        [textField resignFirstResponder];
        [self.locationTextField becomeFirstResponder];

    } else {
        [textField resignFirstResponder];
        [self.bioButton sendActionsForControlEvents: UIControlEventTouchUpInside];
    }

    return NO;
}

- (void) textFieldDidEndEditing: (UITextField *) textField
{
    if (textField == self.fullNameTextField) {
        APP_DELEGATE.fullName = textField.text;

    } else if (textField == self.locationTextField) {
        APP_DELEGATE.location = textField.text;
    }
}

//- (void)didReceiveMemoryWarning {
//    [super didReceiveMemoryWarning];
//    // Dispose of any resources that can be recreated.
//}
//
//#pragma mark - Table view data source
//
//- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
//#warning Incomplete implementation, return the number of sections
//    return 0;
//}
//
//- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
//#warning Incomplete implementation, return the number of rows
//    return 0;
//}
//
///*
//- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
//    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:<#@"reuseIdentifier"#> forIndexPath:indexPath];
//    
//    // Configure the cell...
//    
//    return cell;
//}
//*/
//
///*
//// Override to support conditional editing of the table view.
//- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath {
//    // Return NO if you do not want the specified item to be editable.
//    return YES;
//}
//*/
//
///*
//// Override to support editing the table view.
//- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
//    if (editingStyle == UITableViewCellEditingStyleDelete) {
//        // Delete the row from the data source
//        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
//    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
//        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
//    }   
//}
//*/
//
///*
//// Override to support rearranging the table view.
//- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath {
//}
//*/
//
///*
//// Override to support conditional rearranging of the table view.
//- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath {
//    // Return NO if you do not want the item to be re-orderable.
//    return YES;
//}
//*/

//#pragma mark - Navigation
//
//// In a storyboard-based application, you will often want to do a little preparation before navigation
//- (void)
//prepareForSegue: (UIStoryboardSegue *) segue
//sender:          (id)                  sender
//{
//    // Get the new view controller using [segue destinationViewController].
//    // Pass the selected object to the new view controller.
//}

@end
